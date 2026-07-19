from __future__ import annotations

import argparse
import hashlib
import json
import os
import platform
import subprocess
import sys
import time
import urllib.request
from dataclasses import dataclass
from importlib.metadata import version
from pathlib import Path
from typing import Any

import xa11y

FIXTURE_URL = (
    "https://github.com/mrexodia/dumpulator/releases/download/artifacts/"
    "HarnessFull_x64.dmp"
)
FIXTURE_SHA256 = "595372311c22b9ea33031f9718ab3d7310e825e1ee2a9cfef0e1d0d2fd383bad"
FIXTURE_SIZE = 31_694_576
SCRIPT_DIR = Path(__file__).resolve().parent
CROSS_ROOT = SCRIPT_DIR.parents[1]


@dataclass(frozen=True)
class ControlSpec:
    selector: str
    expected_names: tuple[str, ...]
    minimum_children: int
    minimum_named_children: int = 1
    exercise_scroll: bool = False
    exercise_selection: bool = False


@dataclass(frozen=True)
class TabSpec:
    name: str
    controls: tuple[ControlSpec, ...]


TAB_SPECS = (
    TabSpec(
        "Memory Map",
        (
            ControlSpec(
                "table[name='Memory Map']",
                (
                    "Allocation",
                    "Base",
                    "Size",
                    "Type",
                    "Protect",
                    "Initial",
                    "State",
                    "Info",
                ),
                9,
                exercise_scroll=True,
            ),
        ),
    ),
    TabSpec(
        "Dump",
        (
            ControlSpec(
                "table[name='Dump']",
                ("Address", "Hex", "ASCII"),
                5,
                exercise_scroll=True,
            ),
        ),
    ),
    TabSpec(
        "Disassembly",
        (
            ControlSpec(
                "table[name='Disassembly']",
                (),
                5,
                minimum_named_children=3,
                exercise_scroll=True,
            ),
        ),
    ),
    TabSpec(
        "Threads",
        (
            ControlSpec(
                "table[name='Threads']",
                ("Thread ID", "TEB", "CIP"),
                5,
            ),
            ControlSpec(
                "list[name='Registers']",
                ("RAX", "RSP", "RIP"),
                10,
                exercise_selection=True,
            ),
        ),
    ),
)


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def download_fixture(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".part")
    print(f"Downloading {FIXTURE_URL}")
    request = urllib.request.Request(
        FIXTURE_URL,
        headers={"User-Agent": "x64dbg-accessibility-test"},
    )
    try:
        with urllib.request.urlopen(request, timeout=60) as response:
            with temporary.open("wb") as output:
                while chunk := response.read(1024 * 1024):
                    output.write(chunk)
        temporary.replace(path)
    finally:
        temporary.unlink(missing_ok=True)


def verify_fixture(path: Path, expected_sha256: str | None) -> str:
    if not path.is_file():
        raise FileNotFoundError(path)
    digest = sha256_file(path)
    if expected_sha256 and digest.lower() != expected_sha256.lower():
        raise RuntimeError(
            f"Fixture SHA-256 mismatch: expected {expected_sha256}, got {digest}"
        )
    return digest


def default_minidump_binary() -> Path | None:
    executable = "minidump.exe" if sys.platform == "win32" else "minidump"
    candidates = (
        CROSS_ROOT / "build" / "RelWithDebInfo" / executable,
        CROSS_ROOT / "build" / "Release" / executable,
        CROSS_ROOT / "build" / executable,
        CROSS_ROOT / "build" / "minidump.app" / "Contents" / "MacOS" / "minidump",
        CROSS_ROOT / "build-linux" / executable,
        Path.cwd() / executable,
    )
    return next((candidate for candidate in candidates if candidate.is_file()), None)


def bounds_dict(bounds: Any) -> dict[str, int] | None:
    if bounds is None:
        return None
    return {
        "x": bounds.x,
        "y": bounds.y,
        "width": bounds.width,
        "height": bounds.height,
    }


def element_summary(element: Any) -> dict[str, Any]:
    return {
        "role": element.role,
        "name": element.name,
        "value": element.value,
        "description": element.description,
        "stable_id": element.stable_id,
        "pid": element.pid,
        "actions": list(element.actions),
        "bounds": bounds_dict(element.bounds),
        "states": {
            "enabled": element.enabled,
            "visible": element.visible,
            "focused": element.focused,
            "active": element.active,
            "selected": element.selected,
            "checked": element.checked,
            "expanded": element.expanded,
            "editable": element.editable,
            "focusable": element.focusable,
            "modal": element.modal,
            "required": element.required,
            "busy": element.busy,
        },
        "raw": dict(element.raw),
    }


def snapshot_element(
    element: Any,
    depth: int,
    max_children: int,
    errors: list[str],
) -> dict[str, Any]:
    try:
        result = element_summary(element)
    except Exception as exc:
        return {"error": f"Could not read element: {exc}"}

    result["children"] = []
    result["child_count"] = 0
    if depth <= 0:
        return result

    try:
        children = element.children()
    except Exception as exc:
        errors.append(f"Could not enumerate {element.role}/{element.name!r}: {exc}")
        result["children_error"] = str(exc)
        return result

    result["child_count"] = len(children)
    result["children_truncated"] = len(children) > max_children
    for child in children[:max_children]:
        result["children"].append(
            snapshot_element(child, depth - 1, max_children, errors)
        )
    return result


def descendant_names(snapshot: dict[str, Any]) -> list[str]:
    names = []
    name = snapshot.get("name")
    if name:
        names.append(str(name))
    for child in snapshot.get("children", []):
        names.extend(descendant_names(child))
    return names


def content_signature(snapshot: dict[str, Any]) -> tuple[tuple[str, str, str], ...]:
    result: list[tuple[str, str, str]] = []

    def visit(node: dict[str, Any]) -> None:
        result.append(
            (
                str(node.get("role") or ""),
                str(node.get("name") or ""),
                str(node.get("value") or ""),
            )
        )
        for child in node.get("children", []):
            visit(child)

    visit(snapshot)
    return tuple(result)


def activate_tab(app: Any, name: str) -> None:
    # Qt's Cocoa bridge exposes QTabBar tabs as AXRadioButton on current Qt 6,
    # while other backends and older captures normalize them as `tab`.
    roles = (
        ("radio_button", "tab")
        if sys.platform == "darwin"
        else ("tab", "radio_button")
    )
    last_error: Exception | None = None
    for role in roles:
        locator = app.locator(f"{role}[name='{name}']")
        try:
            locator.wait_visible(timeout=10.0)
            try:
                locator.press()
            except Exception:
                # Some platform bridges expose tabs as selectable but not pressable.
                try:
                    locator.select()
                except Exception:
                    xa11y.input_sim().click(locator.element())
            time.sleep(0.3)
            return
        except Exception as exc:
            last_error = exc
    assert last_error is not None
    raise last_error


def event_summary(event: Any) -> dict[str, Any]:
    target = None
    target_error = None
    try:
        event_target = event.target
        if event_target is not None:
            target = element_summary(event_target)
    except Exception as exc:
        target_error = str(exc)
    return {
        "event_type": event.event_type,
        "app_name": event.app_name,
        "app_pid": event.app_pid,
        "state_flag": event.state_flag,
        "state_value": event.state_value,
        "target": target,
        "target_error": target_error,
    }


def drain_events(subscription: Any, limit: int = 1000) -> list[dict[str, Any]]:
    if subscription is None:
        return []
    events = []
    for _ in range(limit):
        event = subscription.try_recv()
        if event is None:
            break
        events.append(event_summary(event))
    return events


def add_check(
    checks: list[dict[str, str]],
    condition: bool,
    message: str,
    strict: bool,
    required: bool = False,
) -> None:
    if condition:
        status = "PASS"
    elif required or strict:
        status = "FAIL"
    else:
        status = "WARN"
    checks.append({"status": status, "message": message})
    print(f"[{status}] {message}")


def add_known_gap(
    checks: list[dict[str, str]], condition: bool, message: str
) -> None:
    status = "PASS" if condition else "XFAIL"
    checks.append({"status": status, "message": message})
    print(f"[{status}] {message}")


def exercise_scroll(
    process: subprocess.Popen[bytes] | None,
    locator: Any,
    before: dict[str, Any],
    depth: int,
    max_children: int,
    errors: list[str],
) -> dict[str, Any]:
    before_signature = content_signature(before)
    attempts = []
    changed = False
    current = locator.element()

    try:
        current.focus()
    except Exception as exc:
        attempts.append({"action": "focus", "error": str(exc)})
    try:
        xa11y.input_sim().click(current)
    except Exception as exc:
        attempts.append({"action": "click", "error": str(exc)})

    # Backends disagree about wheel sign in practice. Try both directions and
    # stop as soon as the exposed viewport changes.
    after = before
    for delta in (-3, 3):
        try:
            for _ in range(3):
                xa11y.input_sim().scroll(current, dy=delta)
                time.sleep(0.15)
            time.sleep(0.3)
            if process is not None and process.poll() is not None:
                raise RuntimeError(
                    f"minidump exited during scrolling with code {process.returncode}"
                )
            after = snapshot_element(locator.element(), depth, max_children, errors)
            changed = content_signature(after) != before_signature
            attempts.append({"action": "scroll", "dy": delta, "changed": changed})
            if changed:
                break
        except Exception as exc:
            attempts.append({"action": "scroll", "dy": delta, "error": str(exc)})

    return {"changed": changed, "attempts": attempts, "after": after}


def exercise_selection(
    process: subprocess.Popen[bytes] | None,
    locator: Any,
) -> dict[str, Any]:
    result: dict[str, Any] = {"selected": False}
    try:
        children = locator.element().children()
        visible = [
            (index, child)
            for index, child in enumerate(children)
            if child.visible
            and child.bounds is not None
            and child.bounds.width > 0
            and child.bounds.height > 0
        ]
        if not visible:
            raise RuntimeError("No visible child is available for selection")
        index, target = visible[1] if len(visible) > 1 else visible[0]
        result["index"] = index
        result["before"] = element_summary(target)
        try:
            target.focus()
        except Exception as exc:
            result["focus_error"] = str(exc)
        xa11y.input_sim().click(target)
        time.sleep(0.3)
        if process is not None and process.poll() is not None:
            raise RuntimeError(
                f"minidump exited during selection with code {process.returncode}"
            )
        refreshed = locator.element().children()[index]
        result["after"] = element_summary(refreshed)
        result["selected"] = bool(refreshed.selected)
    except Exception as exc:
        result["error"] = str(exc)
    return result


def stop_process(process: subprocess.Popen[bytes]) -> None:
    if process.poll() is not None:
        return
    process.terminate()
    try:
        process.wait(timeout=5)
    except subprocess.TimeoutExpired:
        process.kill()
        process.wait(timeout=5)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Launch the cross-platform minidump tool and inspect its native "
            "accessibility tree through xa11y."
        )
    )
    parser.add_argument(
        "--minidump",
        type=Path,
        help="Path to the minidump executable. Common build paths are auto-detected.",
    )
    parser.add_argument(
        "--dump",
        type=Path,
        help="Dump/PE fixture. Defaults to the pinned HarnessFull_x64.dmp download.",
    )
    parser.add_argument(
        "--attach-pid",
        type=int,
        help="Inspect an already-running minidump process instead of launching one.",
    )
    parser.add_argument(
        "--out",
        type=Path,
        default=SCRIPT_DIR / "captures" / platform.system().lower(),
        help="Directory for JSON and text captures.",
    )
    parser.add_argument("--timeout", type=float, default=30.0)
    parser.add_argument("--depth", type=int, default=4)
    parser.add_argument("--max-children", type=int, default=500)
    parser.add_argument(
        "--no-scroll",
        action="store_true",
        help="Do not synthesize scrolling/model-reset actions.",
    )
    parser.add_argument(
        "--strict",
        action="store_true",
        help="Treat cross-platform discovery warnings as failures.",
    )
    parser.add_argument(
        "--keep-open",
        action="store_true",
        help="Leave a launched minidump process running after capture.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    args.out.mkdir(parents=True, exist_ok=True)

    process: subprocess.Popen[bytes] | None = None
    stdout_handle = None
    stderr_handle = None
    launched = args.attach_pid is None
    fixture_path = args.dump
    fixture_digest = None

    try:
        if launched:
            binary = args.minidump or default_minidump_binary()
            if binary is None:
                raise RuntimeError("Could not auto-detect minidump; pass --minidump")
            binary = binary.resolve()
            if not binary.is_file():
                raise FileNotFoundError(binary)

            if fixture_path is None:
                fixture_path = SCRIPT_DIR / ".cache" / "HarnessFull_x64.dmp"
                if not fixture_path.exists():
                    download_fixture(fixture_path)
                expected_digest = FIXTURE_SHA256
            else:
                expected_digest = None
            fixture_path = fixture_path.resolve()
            fixture_digest = verify_fixture(fixture_path, expected_digest)
            if expected_digest and fixture_path.stat().st_size != FIXTURE_SIZE:
                raise RuntimeError(
                    f"Fixture size mismatch: expected {FIXTURE_SIZE}, "
                    f"got {fixture_path.stat().st_size}"
                )

            stdout_handle = (args.out / "minidump.stdout.log").open("wb")
            stderr_handle = (args.out / "minidump.stderr.log").open("wb")
            environment = os.environ.copy()
            if sys.platform.startswith("linux"):
                environment.setdefault("QT_LINUX_ACCESSIBILITY_ALWAYS_ON", "1")
            process = subprocess.Popen(
                [str(binary), str(fixture_path)],
                cwd=binary.parent,
                env=environment,
                stdout=stdout_handle,
                stderr=stderr_handle,
            )
            pid = process.pid
            print(f"Launched {binary} (PID {pid})")
        else:
            binary = None
            pid = args.attach_pid
            print(f"Attaching to minidump PID {pid}")

        app = xa11y.App.by_pid(pid, timeout=args.timeout)
        print(f"Attached to {app.name!r} (PID {app.pid})")

        try:
            subscription = app.subscribe()
            subscription_error = None
        except Exception as exc:
            subscription = None
            subscription_error = str(exc)
            print(f"[WARN] Event subscription unavailable: {exc}")

        checks: list[dict[str, str]] = []
        errors: list[str] = []
        events: list[dict[str, Any]] = []
        controls: dict[str, Any] = {}

        (args.out / "application.txt").write_text(
            app.dump(max_depth=args.depth), encoding="utf-8"
        )
        (args.out / "application.json").write_text(
            json.dumps(app.tree(max_depth=args.depth), indent=2, ensure_ascii=False),
            encoding="utf-8",
        )

        for tab_spec in TAB_SPECS:
            print(f"\n=== {tab_spec.name} ===")
            try:
                activate_tab(app, tab_spec.name)
            except Exception as exc:
                add_check(
                    checks,
                    False,
                    f"Activate tab {tab_spec.name!r}: {exc}",
                    args.strict,
                    required=True,
                )
                continue

            tab_controls = []
            for control_spec in tab_spec.controls:
                locator = app.locator(control_spec.selector)
                try:
                    element = locator.wait_visible(timeout=10.0)
                    snapshot = snapshot_element(
                        element, args.depth, args.max_children, errors
                    )
                except Exception as exc:
                    add_check(
                        checks,
                        False,
                        f"Find {control_spec.selector}: {exc}",
                        args.strict,
                        required=True,
                    )
                    continue

                add_check(
                    checks,
                    snapshot.get("role")
                    == ("table" if control_spec.selector.startswith("table") else "list"),
                    f"{control_spec.selector} has the expected normalized role",
                    args.strict,
                    required=True,
                )
                child_count = int(snapshot.get("child_count", 0))
                add_check(
                    checks,
                    child_count >= control_spec.minimum_children,
                    f"{control_spec.selector} exposes at least "
                    f"{control_spec.minimum_children} children (got {child_count})",
                    args.strict,
                    required=True,
                )

                names = descendant_names(snapshot)
                named_children = [name for name in names if name != snapshot.get("name")]
                add_check(
                    checks,
                    len(named_children) >= control_spec.minimum_named_children,
                    f"{control_spec.selector} exposes at least "
                    f"{control_spec.minimum_named_children} named descendants "
                    f"(got {len(named_children)})",
                    args.strict,
                    required=True,
                )
                skip_table_headers = (
                    platform.system() == "Darwin"
                    and control_spec.selector.startswith("table")
                )
                for expected_name in control_spec.expected_names:
                    if control_spec.selector.startswith("list"):
                        found = any(
                            name == expected_name
                            or name.startswith(expected_name + " =")
                            for name in names
                        )
                    else:
                        found = expected_name in names
                    message = (
                        f"{control_spec.selector} exposes Name {expected_name!r}"
                    )
                    if skip_table_headers:
                        add_known_gap(
                            checks,
                            found,
                            message
                            + " (known gap in current xa11y snapshots of "
                            "native Qt/AX columns)",
                        )
                    else:
                        add_check(checks, found, message, args.strict)

                bounds = snapshot.get("bounds")
                add_check(
                    checks,
                    bool(bounds and bounds["width"] > 0 and bounds["height"] > 0),
                    f"{control_spec.selector} has non-empty bounds",
                    args.strict,
                )

                try:
                    children = element.children()
                    parent = children[0].parent() if children else None
                    parent_matches = (
                        parent is not None
                        and parent.role == element.role
                        and parent.name == element.name
                    )
                except Exception:
                    parent_matches = False
                add_check(
                    checks,
                    parent_matches,
                    f"First child of {control_spec.selector} points back to the control",
                    args.strict,
                )

                selection_result = None
                if control_spec.exercise_selection:
                    selection_result = exercise_selection(process, locator)
                    add_check(
                        checks,
                        selection_result["selected"],
                        f"Selecting a child of {control_spec.selector} updates its state",
                        args.strict,
                    )

                scroll_result = None
                if control_spec.exercise_scroll and not args.no_scroll:
                    scroll_result = exercise_scroll(
                        process,
                        locator,
                        snapshot,
                        args.depth,
                        args.max_children,
                        errors,
                    )
                    add_check(
                        checks,
                        scroll_result["changed"],
                        f"Scrolling {control_spec.selector} changes its exposed viewport",
                        args.strict,
                    )

                tab_controls.append(
                    {
                        "selector": control_spec.selector,
                        "before": snapshot,
                        "selection": selection_result,
                        "scroll": scroll_result,
                    }
                )
                events.extend(drain_events(subscription))

                safe_name = (
                    control_spec.selector.replace("[", "_")
                    .replace("]", "")
                    .replace("'", "")
                    .replace("=", "_")
                    .replace(" ", "_")
                )
                (args.out / f"{safe_name}.txt").write_text(
                    locator.element().dump(max_depth=args.depth), encoding="utf-8"
                )

            controls[tab_spec.name] = tab_controls

        events.extend(drain_events(subscription))
        if process is not None:
            add_check(
                checks,
                process.poll() is None,
                "minidump remains running after all accessibility actions",
                args.strict,
                required=True,
            )
        report = {
            "platform": {
                "system": platform.system(),
                "release": platform.release(),
                "python": platform.python_version(),
                "xa11y": version("xa11y"),
            },
            "process": {
                "pid": pid,
                "binary": str(binary) if binary else None,
                "fixture": str(fixture_path) if fixture_path else None,
                "fixture_sha256": fixture_digest,
                "exit_code_during_test": process.poll() if process else None,
            },
            "subscription_error": subscription_error,
            "checks": checks,
            "snapshot_errors": errors,
            "controls": controls,
            "events": events,
        }
        (args.out / "report.json").write_text(
            json.dumps(report, indent=2, ensure_ascii=False), encoding="utf-8"
        )

        failures = [check for check in checks if check["status"] == "FAIL"]
        warnings = [check for check in checks if check["status"] == "WARN"]
        expected_failures = [
            check for check in checks if check["status"] == "XFAIL"
        ]
        passed = len(checks) - len(failures) - len(warnings) - len(expected_failures)
        print(
            f"\nWrote {args.out} "
            f"({passed} passed, {len(expected_failures)} expected failures, "
            f"{len(warnings)} warnings, {len(failures)} failures, "
            f"{len(events)} events)"
        )
        return 1 if failures else 0
    except Exception as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 2
    finally:
        if process is not None and not args.keep_open:
            stop_process(process)
        if stdout_handle is not None:
            stdout_handle.close()
        if stderr_handle is not None:
            stderr_handle.close()


if __name__ == "__main__":
    raise SystemExit(main())
