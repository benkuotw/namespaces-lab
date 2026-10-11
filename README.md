# namespaces-lab

Small C programs for practising the three Linux system calls behind containers:
`clone()`, `unshare()` and `setns()`. Each program creates or joins namespaces and prints
which namespaces the process ends up in, so you can see the effect directly.

Written while working through Michael Kerrisk's LWN series
[Namespaces in operation](https://lwn.net/Articles/531114/).

| Program | System call | What it does |
|---|---|---|
| `ns_clone` | `clone()` | Creates a child directly inside new namespaces |
| `ns_unshare` | `unshare()` | Creates new namespaces, then runs a command in them |
| `ns_setns` | `setns()` + `pidfd_open()` | Joins the namespaces of a running process, then runs a command in them |

## Requirements

- Linux 5.8 or later: `pidfd_open()` needs 5.3, and `setns()` with a pidfd needs 5.8
- root, because creating these namespaces requires `CAP_SYS_ADMIN`
- gcc and make

Tested on Debian 13 (kernel 6.12, arm64).

> **Run it in a VM.** The programs mount `/proc` and change the hostname inside the new
> namespaces. These changes are designed to stay inside, but a VM keeps mistakes away from
> your real system.

## Build

```bash
make
```

## Supported namespaces

All three programs use the namespaces listed in the `NS_FLAGS` macro in `ns_util.h`.
I enable each namespace type once I've studied it and have a working example:

- [x] mnt: makes `/` a slave mount (`MS_SLAVE | MS_REC`) so mounts don't leak to the host, then mounts a new `/proc`
- [x] pid: the first process in the new namespace is PID 1
- [x] uts: sets the hostname to `namespaces-lab`
- [ ] cgroup
- [ ] ipc
- [ ] net
- [ ] time
- [ ] user

`/proc` is only mounted when both mnt and pid are enabled. A new `/proc` only makes sense for
a new PID namespace, and mounting it without a new mount namespace would replace the host's
`/proc`.

## Usage

All examples run as root (`#` is the root prompt).

Each program prints which namespace each process is in, as `/proc/self/ns` symlinks such as
`mnt:[4026532236]`. Two processes are in the same namespace when the numbers match, as long as
both are running (see [What I learned](#what-i-learned)).

### ns_clone: create a child in new namespaces

```console
# ./ns_clone &
```

The child sets up the namespaces, prints them and sleeps for 60 seconds, which gives you time
to join it with `ns_setns`.

```
# ./ns_clone &
./ns_clone[7316]: /proc/7316/ns; debian-vm
./ns_clone[7316]: mnt:[4026531841]
./ns_clone[7316]: pid:[4026531836]
./ns_clone[7316]: uts:[4026531838]
./ns_clone[7316]: created a child[7317] for the new namespaces
./ns_clone[1]: /proc/1/ns; namespaces-lab
./ns_clone[1]: mnt:[4026532236]
./ns_clone[1]: pid:[4026532238]
./ns_clone[1]: uts:[4026532237]
```

The parent is PID 7316 in the host namespaces. The child is PID 7317 on the host but PID 1
inside, with its own mnt, pid and uts namespaces and the hostname `namespaces-lab`.

Process tree:

```
ns_clone              host namespaces; waits
└── child             PID 1 in the new namespaces; sets up mnt/uts, sleeps 60 s
```

### ns_setns: join a running process's namespaces

```console
# ./ns_setns PID CMD [ARGS...]
```

Use the child PID printed by `ns_clone`. The program opens a pidfd for that process and
joins all namespaces in `NS_FLAGS` with a single `setns()` call. It doesn't change anything
inside the namespaces it joins.

```
# ./ns_setns 7317 /bin/bash
./ns_setns[7320]: /proc/7320/ns; debian-vm
./ns_setns[7320]: mnt:[4026531841]
./ns_setns[7320]: pid:[4026531836]
./ns_setns[7320]: uts:[4026531838]
./ns_setns[7320]: created a child[7321] for the namespaces to join
./ns_setns[7321]: created a child[7322] for the PID namespace to join
./ns_setns[2]: /proc/2/ns; namespaces-lab
./ns_setns[2]: mnt:[4026532236]
./ns_setns[2]: pid:[4026532238]
./ns_setns[2]: uts:[4026532237]
```

The grandchild is PID 2 inside the `ns_clone` child's PID namespace, and its namespace
numbers match that child's.

Process tree:

```
ns_setns              host namespaces; waits
└── child             setns(pidfd): joined mnt/uts, still in the host PID namespace; waits
    └── grandchild    inside the target's PID namespace; execs the command
```

### ns_unshare: create namespaces and run a command

```console
# ./ns_unshare CMD [ARGS...]
```

```
# ./ns_unshare /bin/bash
./ns_unshare[7331]: /proc/7331/ns; debian-vm
./ns_unshare[7331]: mnt:[4026531841]
./ns_unshare[7331]: pid:[4026531836]
./ns_unshare[7331]: uts:[4026531838]
./ns_unshare[7331]: created a child[7332] for new namespaces
./ns_unshare[7332]: created a child[7333] for the new PID namespace
./ns_unshare[1]: /proc/1/ns; namespaces-lab
./ns_unshare[1]: mnt:[4026532236]
./ns_unshare[1]: pid:[4026532238]
./ns_unshare[1]: uts:[4026532237]
```

The grandchild is PID 1 in the new PID namespace and runs `/bin/bash` with the new hostname.

Process tree:

```
ns_unshare            host namespaces; waits
└── child             unshare(): in the new mnt/uts namespaces, still in the host PID namespace; waits
    └── grandchild    PID 1 in the new PID namespace; sets up mnt/uts, execs the command
```

In all three programs, a parent waits for its child and exits with the child's exit status, so
`$?` in your shell is the command's exit status.

## What I learned

- `unshare()` and `setns()` with `CLONE_NEWPID` don't move the calling process into the PID
  namespace; only its children are created there. That's why `ns_unshare` and `ns_setns` fork
  once more before running the command, while `ns_clone` doesn't: `clone()` creates the child
  directly inside the new PID namespace.
- Namespace inode numbers are reused. A namespace is freed once nothing refers to it any
  more (no process inside it, no open fd, no bind mount), and the next namespace created can
  get the same number. In the examples above, `ns_unshare` shows the same numbers as
  `ns_clone` because the `ns_clone` child had already exited. Run `ns_unshare` while the child
  is still sleeping and the numbers differ. So matching numbers mean the same namespace only
  when both processes are alive at the same time.

## Related

While studying mount namespaces, I found and fixed a broken `mount()` example in the
`mount_namespaces(7)` man page. The fix was applied upstream as
[e7afce1d563e](https://git.kernel.org/pub/scm/docs/man-pages/man-pages.git/commit/?id=e7afce1d563e).

## References

- Michael Kerrisk, [Namespaces in operation](https://lwn.net/Articles/531114/), LWN.net
- Man pages: [namespaces(7)](https://man7.org/linux/man-pages/man7/namespaces.7.html),
  [clone(2)](https://man7.org/linux/man-pages/man2/clone.2.html),
  [unshare(2)](https://man7.org/linux/man-pages/man2/unshare.2.html),
  [setns(2)](https://man7.org/linux/man-pages/man2/setns.2.html),
  [pidfd_open(2)](https://man7.org/linux/man-pages/man2/pidfd_open.2.html),
  [pid_namespaces(7)](https://man7.org/linux/man-pages/man7/pid_namespaces.7.html),
  [mount_namespaces(7)](https://man7.org/linux/man-pages/man7/mount_namespaces.7.html)

## License

MIT. See [LICENSE](LICENSE).
