#!/usr/bin/env python3
"""GUI for gamma and brightness controls."""
import os
import subprocess
import sys
from tkinter import Tk, Scale, HORIZONTAL, Button, Label, Frame, LEFT, RIGHT

os.environ.setdefault("DISPLAY", ":0")
if "XAUTHORITY" not in os.environ:
    default_xauth = os.path.expanduser("~/.Xauthority")
    if os.path.exists(default_xauth):
        os.environ["XAUTHORITY"] = default_xauth

try:
    out = subprocess.check_output(
        ["xrandr", "--query"],
        text=True,
        stderr=subprocess.DEVNULL,
    )

except subprocess.CalledProcessError as e:
    print("ERROR: xrandr returned non-zero exit status:", e, file=sys.stderr)
    sys.exit(1)

except FileNotFoundError as e:
    print("ERROR: xrandr not found:", e, file=sys.stderr)
    sys.exit(1)

lines = [l for l in out.splitlines() if " connected" in l]
if not lines:
    print("ERROR: no connected output found via xrandr", file=sys.stderr)
    sys.exit(1)

PRIMARY = None
for l in lines:
    if " primary " in l:
        PRIMARY = l.split()[0]
        break
OUTPUT = PRIMARY or lines[0].split()[0]


def clamp(v, lo, hi):
    try:
        fv = float(v)
    except Exception:
        return lo
    return max(lo, min(hi, fv))

def apply_gamma(g_val):
    v = clamp(g_val, 0.5, 10.0)
    gamma_str = f"{v}:{v}:{v}"
    subprocess.call(["xrandr", "--output", OUTPUT, "--gamma", gamma_str])

def apply_brightness(b_val):
    v = clamp(b_val, 0.1, 2.0)
    subprocess.call(["xrandr", "--output", OUTPUT, "--brightness", str(v)])

def revert(reset_sliders=True):
    subprocess.call(["xrandr", "--output", OUTPUT, "--gamma", "1:1:1"])
    subprocess.call(["xrandr", "--output", OUTPUT, "--brightness", "1"])
    if reset_sliders:
        try:
            g.set(100)
            b.set(100)
        except Exception:
            pass

def revert_and_exit():
    revert(reset_sliders=True)
    root.destroy()

root = Tk()
root.title("x11gammagui")

Label(root, text=f"Display output: {OUTPUT}").pack(padx=10, pady=(8,0))

frame = Frame(root)
frame.pack(padx=10, pady=8)

Label(frame, text="Gamma (0.5 to 10.0, 1.0 default)").pack()
g = Scale(frame, from_=50, to=1000, orient=HORIZONTAL, length=420)
g.set(100)
g.pack(pady=(0,8))

Label(frame, text="Brightness (0.1 to 2.0, 1.0 default)").pack()
b = Scale(frame, from_=10, to=200, orient=HORIZONTAL, length=420)
b.set(100)
b.pack(pady=(0,8))

def on_gamma_change(val):
    apply_gamma(float(val) / 100.0)

def on_brightness_change(val):
    apply_brightness(float(val) / 100.0)

g.config(command=on_gamma_change)
b.config(command=on_brightness_change)

btnframe = Frame(root)
btnframe.pack(fill="x", padx=10, pady=10)

btn = Button(
    btnframe,
    text="Apply",
    command=lambda: (
        apply_gamma(g.get() / 100.0),
        apply_brightness(b.get() / 100.0),
    ),
)
btn.pack(side=LEFT, padx=5)

btn = Button(
    btnframe,
    text="Preset 1.5 / +10%",
    command=lambda: (
        g.set(150),
        b.set(110),
        apply_gamma(1.50),
        apply_brightness(1.10),
    ),
)
btn.pack(side=LEFT, padx=5)
btn = Button(
    btnframe,
    text="Preset 2.0 / +20%",
    command=lambda: (
        g.set(200),
        b.set(120),
        apply_gamma(2.00),
        apply_brightness(1.20),
    ),
)
btn.pack(side=LEFT, padx=5)
Button(btnframe, text="Revert & Exit", command=revert_and_exit).pack(side=RIGHT, padx=5)
Button(btnframe, text="Revert", command=lambda: revert(reset_sliders=True)).pack(side=RIGHT, padx=5)

root.mainloop()
