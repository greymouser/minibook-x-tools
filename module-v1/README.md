Calibrate signed
```bash
echo 1 | sudo tee /sys/kernel/chuwi-minibook-x-tablet-mode/calibrate_signed
```

Force the generation of SW_TABLET_MODE off
```bash
echo 0 | sudo tee /sys/kernel/chuwi-minibook-x-tablet-mode/force
```
* Use `1` to force enable it, and `-1` for no forced state (the default).
