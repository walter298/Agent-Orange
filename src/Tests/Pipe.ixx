export module Chess.Tests:Pipe;

import std;

namespace chess {
	struct OSPipe {
		virtual void write(std::string_view command) = 0;
		virtual std::string read(std::string_view expectedBegin) = 0;
		virtual ~OSPipe() {}
	};

	export class Pipe {
	private:
		std::unique_ptr<OSPipe> m_osPipe = nullptr;
	public:
		Pipe();
		
		void write(std::string_view command) const;
		std::string read(std::string_view expectedBegin) const;
	};
}