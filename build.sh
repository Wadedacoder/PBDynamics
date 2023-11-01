cd ./build
# add a flag to do cmake
while getopts ":c" opt; do
  case $opt in
    c)
      echo "cmake"
      cmake ..
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
  esac
done
make
cd ..