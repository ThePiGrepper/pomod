# pomod

The pomodoro daemon.

pomod implements the [Pomodoro Technique][pomodoro-site], as designed by Francesco Cirillo.

## Running

As it's name implies, pomod is implemented as a daemon, that means that the server-side process runs in
the background, waiting for a request, usually sent by the client-side process.

```shell
pomod start
pomod halt
pomod info
pomod kill
```

## Building

Install dependencies:
* libnotify
* meson (build dependency)
* scdoc (build dependency, optional for man pages)

Then run:

```shell
meson build
ninja -C build
build/pomod
```

## License

MIT

[pomodoro-site]: https://francescocirillo.com/pages/pomodoro-technique
