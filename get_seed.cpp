static int get_seed(int argc, char** argv) {
  if (argc != 2) {
    printf("Usage: %s <random-seed>\n", argv[0]);
    exit(1);
  }
  
  return atoi(argv[1]);
}
