// REQUIRES: system-linux, lldb
//
// RUN: %dexter --fail-lt 1.0 -w \
// RUN:     --builder 'clang' --debugger 'lldb' --cflags "-O0 -glldb" -- %s

class A {
public:
	A() : zero(0), data(42) { // DexLabel('ctor_start')
	}
private:
	int zero;
	int data;
};

int main() {
	A a;
	return 0;
}


/*
DexExpectProgramState({
	'frames': [
		{
			'location': {
				'lineno': 'ctor_start'
			},
			'watches': {
				'*this': {'is_irretrievable': False}
			}
		}
	]
})
*/

