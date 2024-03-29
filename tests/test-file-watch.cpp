#include <string>
#include <regex>
#include <iostream>
#include "filewatch.hpp"
#include "utilstring.h"
#include "utilfile.h"

/**
 * @see https://github.com/ThomasMonkman/filewatch
 */
int main(int argc, char **argv) {
	filewatch::FileWatch<std::string> watch(".", 
	std::regex(".*\.json"),
	[](const std::string& path, const filewatch::Event change_type) {
		std::cout << path << ": ";
		switch (change_type)
		{
		case filewatch::Event::added:
			std::cout << "The file was added to the directory." << '\n';
			break;
		case filewatch::Event::removed:
			std::cout << "The file was removed from the directory." << '\n';
			break;
		case filewatch::Event::modified:
			std::cout << "The file was modified. This can be a change in the time stamp or attributes." << '\n';
			break;
		case filewatch::Event::renamed_old:
			std::cout << "The file was renamed and this is the old name." << '\n';
			break;
		case filewatch::Event::renamed_new:
			std::cout << "The file was renamed and this is the new name." << '\n';
			break;
		};
	}
	);

	const std::string fn("test-file-watch.tmp.json");
	string2file(fn, "1");
	util::rmFile(fn);
	sleep(1);
}
