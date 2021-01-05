# Vi port for Windows
Port is based on fully stripped busybox-W32 version. No external dependencies. Zig compile compatible

Check https://github.com/rmyorston/busybox-w32/ for more info about libraries used

### But why?
I did this as exploration of what needs to be done to do a terminal editor on windows, in the style of kilo, without any dependencies and compile it with Zig.
Generally there are really no other Windows based terminal editor projects, made without dependencies, on github. There is also no standalone classic Vi Windows port, so i thought that stripping down busybox version to the minimum would be a neat task.

### Compilation:
Just run: 

	zig build-exe -lc vi.c compare_string_array.c env.c hash_md5_sha.c isaac.c mingw.c platform.c ptr_to_globals.c read.c termios.c winansi.c xfuncs_printf.c other_functions.c -I ".\custom_include" -target x86_64-windows-gnu


Or:

	zig build // to be added

I have not checked GCC compiler, but it should easily compile there as well as long as you have MINGW libraries in the path.
If i check GCC compiler, i'll definitely update this section.

### TO DO:
- make it cross compile on linux
- clean the busybox based libraries even more
- enable more functionalities
