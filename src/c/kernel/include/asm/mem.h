void *align(void *ptr, unsigned int alignment) {
    return (void *) ((unsigned long) ptr & -alignment);
}
