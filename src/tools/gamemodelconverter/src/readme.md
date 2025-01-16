The main goal of the project is to create a set of tools that allow the conversion between different file formats used
in 3D modeling for game development. Specifically, the tools will focus on converting **W3D** files (binary format) into
**W3X** (XML format), and also facilitate conversion between W3D and other formats like **Blend** (Blender files) and *
*Max** (3ds Max files). Here's a breakdown of the key points:

1. **W3D to W3X Conversion**: The first task is to convert binary W3D files into a text-based W3X format. This would
   allow better compatibility and transparency because W3X is an XML format, which is easier to parse and edit compared
   to the binary W3D format.

2. **Thyme as the Starting Point**: The development of the **W3D to W3X** conversion tool will begin by using **existing
   code in Thyme**, which will help maintain consistency and compatibility across tools and workflows.

3. **Maintaining Consistency with Mod Builder**: Once W3X files are created, the Mod Builder tool will allow conversion
   between W3D, Blend, and Max files. The goal is to ensure that these conversions are **non-destructive**, meaning that
   converting a Blend or Max file back to W3D should not introduce any errors or issues. The **Blend to W3D** conversion
   is already supported by Mod Builder, but there are concerns about potential issues with the Blender plugin causing
   data loss on import.

4. **W3D as the "File of Truth"**: Throughout this process, the W3D file is considered the authoritative format. W3X is
   a **faithful representation** of the W3D file, while Blend and Max files are **non-destructive** representations,
   meaning they can be used for editing and modeling without corrupting or altering the original W3D file.

In summary, the project involves creating a reliable pipeline that can convert between W3D, W3X, Blend, and Max files,
using W3D as the reference format and leveraging existing tools like Thyme and Mod Builder.