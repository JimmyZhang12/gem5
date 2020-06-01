# McPAT XML Auto Gen

This project takes a GEM5 statistics file and config file, and builds
a system that can be serialized to XML for use with McPAT. Although it
is configured to work with GEM5 as the cycle accurate simulator, the
utility is designed to be easily converted and extended to fit other
simulators.

### Structure

The Gem5 device tree and the McPAT device tree are not the same, thus
a conversion from one tree to another is required. The McPAT tree is
as follows:

- Root
  - System
    - Core [0,...,*n*]
      - BranchPredictor
      - Branch Target Buffer (BTB)
      - ICache Translation Lookahead Buffer (TLB)
      - ICache
      - DCache TLB
      - DCache
    - L2 Cache [0,...,*n*]
    - L1 Directory [0,...,*n*]
    - L2 Directory [0,...,*n*]
    - L3 Cache
    - Network on Chip (NoC)
    - Memory Controller (MC)
    - Network Interface (NIU)
    - PCIe
    - Flash Controller

### Versioning and Features

Version 1.0 - Initial code, converts device tree, supports parsing for
multi core gem5 stats with heterogenous cores, and heterogenus private
L2 caches.

### Future Work

In the future this needs to be expanded on and improved on. There are
more xml parameters and stats that can be specified to improve the
power approximations, also there should be RUBY coherent caches
supported with NoCs for each level of coherent cache.

### Authors

* **Andrew Smith** - *Base classes, system class, serialization* -
[atsmith3](https://github.com/atsmith3)

See also the list of
[contributors](https://github.com/atsmith3/predict-T/graphs/contributors)
who participated in this project.

### License

This project is licensed under the MIT License - see the
[LICENSE.md](LICENSE.md) file for details.
