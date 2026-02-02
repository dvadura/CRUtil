# CRUtil TODO

## Completed

- [x] Rename `bigint.h` → `bigint128.h`, create backward-compat shim
- [x] Create `bigint256.h` with full `uint256_t` implementation
- [x] Create `test_bigint256.cpp` test suite (22 tests, 203 assertions)
- [x] Update documentation (README, Reference, CHANGELOG)

## Big-Endian Validation via Docker + QEMU

- [ ] Verify Docker Desktop is installed and running
- [ ] Enable multiarch QEMU support:
      ```
      docker run --rm --privileged multiarch/qemu-user-static --reset -p yes
      ```
- [ ] Identify source paths for `crstring.cpp` and `test_main.cpp` (needed for in-container compilation)
- [ ] Run s390x (big-endian) container with project mounted:
      ```
      docker run --rm -it -v $(pwd):/work s390x/ubuntu:22.04 bash
      ```
- [ ] Inside the container, install toolchain and compile from source:
      ```
      apt-get update && apt-get install -y g++
      cd /work
      g++ -std=gnu++17 -g -DDEBUG=1 -I Source/include -I Test \
          -c Test/test_bigint.cpp -o /tmp/test_bigint.o
      g++ -std=gnu++17 -g -I Source/include -I Test \
          -c Test/test_main.cpp -o /tmp/test_main.o
      g++ -std=gnu++17 -g -I Source/include \
          -c Source/crstring.cpp -o /tmp/crstring.o
      g++ -o /tmp/test_bigint /tmp/test_main.o /tmp/test_bigint.o /tmp/crstring.o
      /tmp/test_bigint
      ```
- [ ] Verify all 56+ test cases pass on big-endian
- [ ] Fix any failures discovered by BE run
