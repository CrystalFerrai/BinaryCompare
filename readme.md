## BinaryCompare

Simple library for quickly comparing two files to determine of they are binary equal. Uses memory mapping to provide better performance than typical file reading.

If you are seeking a command line tool for binary file comparison on Windows, check out [BComp](https://github.com/CrystalFerrai/BComp). It makes use of this library.

## Usage

Only a single function is exported by the library:

```
int CompareFiles(const wchar_t* aPath, const wchar_t* bPath)
```

It returns 0 if it found any differences, 1 if the files are identical, or -1 if there were problems reading either of the files.

**Using from .NET**

You can import the library's single function into .NET like this:

```
[DllImport("BinaryCompare.dll")]
public static extern int CompareFiles([MarshalAs(UnmanagedType.LPWStr)] string aPath, [MarshalAs(UnmanagedType.LPWStr)] string bPath);
```

## Releases

Release can be found [here](https://github.com/CrystalFerrai/BinaryCompare/releases).

## Building

To build from source, just open the SLN file in Visual Studio 2022 and build it. For older versions of Visual Studio, you should be able to create a solution containing the library's single source file and build that.

## Platforms Supported

**Windows Only**

BinaryCompare uses Windows-specific memory mapping APIs. Other platforms have similar APIs, but only Windows has been implemented. Therefore, this library will not work on other platforms.