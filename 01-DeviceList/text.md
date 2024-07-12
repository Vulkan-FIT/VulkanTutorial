Vulkan is a low-level, low-overhead API for 3D graphics and computing. Currently, it is one of the most important APIs in its area.

<details><summary>Vulkan history and design</summary>
  Vulkan 1.0 was released in 2016. It is almost 25 years after the first version of OpenGL.
  On the beginning of 90', graphics cards were - if we simplify it - only a piece of memory with monitor output.
  Whatever was written to the memory appeared on the screen. Today in 2024, a main stream graphics card is programmable, massively parallel compute unit.
  When comparing computing power, it might outperform tens, hundreds, or even thousands of traditional processors in extreme cases.
  Vulkan is designed with the focus on effective use of the performance potential that is hidden in the graphics cards of today.

  The other view: 25 years before Vulkan 1.0, almost all computers had only one processor capable of executing a single thread at a time.
  This corresponds to a single active OpenGL context per thread. This was logical design for these old times, but it does not fit well in our reality.
  Today, standard computer contains multi-core processor capable of executing many threads simultaneously.
  Executing of tens of threads in parallel is not an exception.
  No wonder that Vulkan is designed to be able to take advantage of multi-threaded programming and parallel processing by many cores of the processor.
  And not only multi-core processing is in Vulkan design focus, but also multiple graphics cards can be handled by Vulkan natively.

  OpenGL has high overhead for some operations and its driver is very complex.
  On the other side, Vulkan is low-level API with low overhead and relatively simple driver.
  Simple driver usually results in much less driver bugs and better driver and system stability.

  OpenGL is platform neutral, but faced difficulties anyway. On macOS, it was always number of versions behind the standard.
  On mobile devices, usually only OpenGL ES was supported.
  When looking on Vulkan, it is supported on macOS (through MoltenVk) and majority of modern tablets and mobile phones.  
</details>
