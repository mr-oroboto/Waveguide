# Waveguide

An OpenGL-based tool to visualise realtime radio spectrum as acquired by an SDR
device. It uses the `gr-osmosdr` GNU Radio block, so it should support anything
that it does and has been tested with an RTL-SDR and AirSpy Mini.

Supports multiple visualisations including a standard linear FFT, but includes
experimental views that can help to visual large chunks of spectrum and how
they evolve over time.

## Dependencies

* GNU Radio (with the [gr-osmosdr](https://github.com/osmocom/gr-osmosdr) block)
* [Insight](https://github.com/mr-oroboto/Insight)
* [libsdl2](https://www.libsdl.org/)
* libglew
* libglm
* libfreetype6

