Import("env")
import os

pioenv = env.subst("$PIOENV")
clib_dir = os.path.join(env.subst("$PROJECT_LIBDEPS_DIR"), pioenv, "U8g2", "src", "clib")

env.Append(CPPPATH=[clib_dir])

src_filter = [
    "+<*.c>",
    "-<mui*.c>",
    "-<u8x8_d_*.c>",
    "+<u8x8_d_ssd1306_128x64_noname.c>",
]

env.BuildSources(os.path.join("$BUILD_DIR", "u8g2"), clib_dir, src_filter)
