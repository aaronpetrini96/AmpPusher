"""Capture configuration: where each pedal's renders live and which knob settings they cover."""

from pathlib import Path

import soundfile as sf

REPO_ROOT = Path(__file__).resolve().parent.parent
RECORDINGS_DIR = REPO_ROOT / "recordings"

SAMPLE_RATE = 48_000
INPUT_FILE = RECORDINGS_DIR / "input.wav"
SWEEP_FILE = RECORDINGS_DIR / "sources" / "T3K-sweep-v3.wav"
SWEEP_TO_DI_GAP_SECONDS = 1.0
VALIDATION_FRACTION = 0.2

DRIVE_SETTINGS = {0.0: "000", 0.25: "025", 0.5: "050", 0.75: "075", 1.0: "100"}
BRIGHT_SETTINGS = {0.0: "05", 1.0: "10"}

PEDAL_DIRS = {
    "ts808": RECORDINGS_DIR / "ts808",
    "sd1": RECORDINGS_DIR / "sd1",
}


def di_start_sample() -> int:
    """First sample of the DI section in the input file, which follows the sweep and a short gap."""
    return sf.info(SWEEP_FILE).frames + int(SWEEP_TO_DI_GAP_SECONDS * SAMPLE_RATE)


def render_file(pedal: str, drive: float, bright: float) -> Path:
    """Path of the render captured at the given drive and bright settings."""
    return PEDAL_DIRS[pedal] / f"drive_{DRIVE_SETTINGS[drive]}_tone_{BRIGHT_SETTINGS[bright]}.wav"
