#pragma once
#include <ctime>

using namespace std;

namespace nNet {

	class Benchmark {
	private:
		time_t _t;

	public:
		Benchmark() {
			Reset();
		}

		float TotalSeconds() {
			time_t now = clock();
			return (float)(now - _t) / CLOCKS_PER_SEC;
		}

		void Reset() {
			_t = clock();
		}
	};
}
