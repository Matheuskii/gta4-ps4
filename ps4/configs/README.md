# Console config

`native.toml` is the config in use with build CX (2026-11-04): it goes to the
console at `/data/LibertyRecomp/native.toml`. Draw distance and densities are at
0.5 (0.6/0.75 looks fuller for 1-2 FPS less); the sun shadow map is redrawn every
second frame. `ps4/HANDOFF-2026-11-02.md` has the history of every setting.

## PS4 Slim

The experimental [Slim profile](slim/native.toml) uses settings available in the
`v0.0.1` release. It lets draw distance range from 0.35 to 0.5 as frame times
change, sets ambient traffic, parked cars and pedestrians to 0.35, and updates
sun shadows every third frame. Expect a less crowded city, objects appearing
closer in heavy scenes, and less frequent shadow updates.

Close the game, back up your current `/data/LibertyRecomp/native.toml`, copy
`slim/native.toml` to that same path and relaunch. To compare profiles, drive
the same route from the docks to Roman's apartment in each run and check
the on-screen FPS and audio dropouts. Restore your backup and restart to switch
back. Console performance with this profile still needs to be measured.
