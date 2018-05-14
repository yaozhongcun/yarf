#~/software/cppcheck-1.56/cppcheck  --enable=style,performance,portability,unusedFunction,missingInclude --includes-file=3rdparty/include.txt  src 2>/tmp/cppcheck.txt 
#cat /tmp/cppcheck.txt | grep "^\[3rd" -v | grep "\(information\)" -v


#root 必须是全路径，否则会header guard错误

shell_dir=`dirname $0`
cd $shell_dir
shell_dir=`pwd`
#echo $shell_dir
echo -e "\033[32mcheck in ${shell_dir} \033[0m"


cd include/yarf
for i in `ls */*.h *.h`; do
	if [[ "$i" =~ ".pb." ]]; then
		echo "skip $i"
	else
		#"${shell_dir}/cpplint.py" --filter=-runtime/references --root=${shell_dir}/include/yarf/ $i 
		"${shell_dir}/cpplint.py" --filter=-runtime/references --root=include $i

	fi
done
cd - 

cd src/yarf
for i in `ls */*.cc */*.h`; do
	if [[ "$i" =~ ".pb." ]]; then
		echo "skip $i"
	else
		"${shell_dir}/cpplint.py" --filter=-runtime/references --root=src $i 
		#./cpplint.py --filter=-runtime/references --root=/home/yaozhongcun/common_lib/yarf/src/libyarf $i 
	fi
done
cd - 

echo -e "\033[32mcompile in ${shell_dir} \033[0m"
mkdir build
cd build
cmake ..
make
cd -

#cd src/
#~/common_lib/lib4g-0.2/cpplint.py  --root=include */*/*.cc
#cd -



