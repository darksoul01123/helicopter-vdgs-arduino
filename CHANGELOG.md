# Change Ledger (`helicopter-vdgs-arduino`)

Every feature, pin change, or firmware update is tracked as a separate entry and Git commit so you can review history or roll back anytime.

## How to Roll Back to Any Version
```powershell
git log --oneline                  # View all saved versions
git checkout <commit-hash> -- .    # Restore files from a specific version
```

---

## Ledger

### `v1.0.0` — Initial Hardware-Tested Release (`1427785`)
- **Date:** 2026-11-26
- **Summary:** Finalized Helicopter VDGS & Ultrasonic Proximity System calibrated from live `COM14` hardware tests.
- **Hardware Pinout:** `TRIG=D9`, `ECHO=D10`, `Green=D5`, `Yellow=D6`, `Red=D7`, `Buzzer=D8`, `Button=D2`, `OLED=A4/A5`.
- **Files:** `helicopter-vdgs-arduino.ino`, `WIRING_GUIDE.md`, `vdgs_breadboard_wiring.html`, `upload_and_monitor.ps1`, `upload_and_monitor.bat`, `README.md`.
