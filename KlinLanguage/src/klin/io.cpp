#include "io.h"
#include <Windows.h>
#include <iostream>
#include <fstream>

namespace klin {

	std::wstring to_wstring(const std::string& str) {
		if (str.empty()) return L"";
		int slength = static_cast<int>(str.length()) + 1;
		int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, nullptr, 0);
		if (len <= 0) return L"";

		std::wstring r(len - 1, L'\0');
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, &r[0], len);
		return r;
	}
	std::string read_file(const char* filepath) {
		std::ifstream file(filepath, std::ios::binary | std::ios::ate);
		if (!file.is_open())
			return "";

		std::streamsize fileSize = file.tellg();
		file.seekg(0, std::ios::beg);

		std::string content(fileSize, '\0');
		if (file.read(&content[0], fileSize)) {
			return content;
		}

		return "";
	}

	std::string get_line_file(const char* filepath, unsigned int lineNumber) {
		std::ifstream inputFile(filepath);
		if (!inputFile.is_open() || lineNumber == 0)
			return "";

		std::string line;
		unsigned int currentLine = 1;

		while (std::getline(inputFile, line)) {
			if (currentLine == lineNumber) {
				size_t pos = 0;
				while ((pos = line.find('\t', pos)) != std::string::npos) {
					line.replace(pos, 1, "    ");
					pos += 4;
				}
				return line;
			}
			currentLine++;
		}
		return "";
	}

	void print_diagnostic(const char* filepath, const klin::Diagnostic& diag) {
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		size_t spaceNeeded = std::to_string(diag.line).size() + 2;

		// En-tête de l'erreur : (file:line:col, error): message
		SetConsoleTextAttribute(hConsole, 0x0C); // Rouge
		std::cout << "(" << filepath << ":" << diag.line << ":" << diag.pos << ", error): ";
		SetConsoleTextAttribute(hConsole, 0x0F); // Blanc brillant
		std::cout << diag.message << "\n";

		// Ligne précédente pour donner du contexte
		if (diag.line > 1) {
			std::string prevLineNum = std::to_string(diag.line - 1);
			SetConsoleTextAttribute(hConsole, 0x0A); // Vert
			std::cout << " " << prevLineNum;
			for (size_t i = 0; i < (spaceNeeded - prevLineNum.size() - 1); ++i) std::cout << " ";
			std::cout << "| ";
			SetConsoleTextAttribute(hConsole, 0x08); // Gris
			std::cout << get_line_file(filepath, diag.line - 1) << "\n";
		}

		// Ligne courante où se trouve l'erreur
		std::string currentLineNum = std::to_string(diag.line);
		SetConsoleTextAttribute(hConsole, 0x0A); // Vert
		std::cout << " " << currentLineNum;
		for (size_t i = 0; i < (spaceNeeded - currentLineNum.size() - 1); ++i) std::cout << " ";
		std::cout << "| ";
		SetConsoleTextAttribute(hConsole, 0x07); // Blanc normal
		std::cout << get_line_file(filepath, diag.line) << "\n";

		// Curseur '^' ciblant la colonne
		SetConsoleTextAttribute(hConsole, 0x0A);
		for (size_t i = 0; i < spaceNeeded; ++i) std::cout << " ";
		std::cout << "| ";

		// Alignement précis selon le numéro de colonne (1-based)
		unsigned int caret_offset = (diag.pos > 0) ? (diag.pos - 1) : 0;
		for (unsigned int i = 0; i < caret_offset; ++i) std::cout << " ";

		SetConsoleTextAttribute(hConsole, 0x0C); // Red underlying
		unsigned int underline_length = (diag.length > 0) ? diag.length : 1;
		for (unsigned int i = 0; i < underline_length; ++i) std::cout << "^";

		std::cout << "\n\n";
		SetConsoleTextAttribute(hConsole, 0x0F); // Reset couleur
	}
}