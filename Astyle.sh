astyle --options=./arilux.astylerc --recursive *.h
astyle --options=./arilux.astylerc --recursive *.cpp
astyle --options=./arilux.astylerc --recursive *.hpp


find . -type f -name "*.orig" -exec rm {} \;