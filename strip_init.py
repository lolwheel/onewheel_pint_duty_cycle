Import("env")

env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.elf",
    env.VerboseAction(" ".join([
        "$OBJCOPY", "-O", "binary", "-R.fini", "-R.init",
        "$BUILD_DIR/${PROGNAME}.elf", "$BUILD_DIR/${PROGNAME}_noinit.bin"
    ]), "Building $BUILD_DIR/${PROGNAME}_noinit.bin")
)