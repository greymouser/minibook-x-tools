mkdir -p bin
mkdir -p dsl
pushd bin
  sudo acpidump -b
popd

pushd dsl
  for f in `ls -1 ../bin/*.dat`; do
    iasl -d $f
  done 
popd
