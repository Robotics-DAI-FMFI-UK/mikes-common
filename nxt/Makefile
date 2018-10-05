all: NXTOperator test

test: TestMonoBrick

NXTOperator: NXTOperator.cs
	mcs NXTOperator.cs -out:NXTOperator -r:lib/MonoBrick

TestMonoBrick: tests/TestMonoBrick.cs
	mcs tests/TestMonoBrick.cs -out:TestMonoBrick -r:lib/MonoBrick

clean:
	rm -f TestMonoBrick NXTOperator
	rm -f tests/*.exe
