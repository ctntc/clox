#include <lox/syntax/token.hpp>

#include <cstdio>
#include <cstdlib> 
#include <exception>
#include <format>
#include <fstream>
#include <iterator>
#include <lox/syntax/location.hpp>
#include <print>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
  /**
   *
   * @param filename The path to the file to read.
   * @return A Unicode string containing the file's contents.
   */
  auto read_to_wstring(const char filename[]) noexcept(false)
    -> std::wstring {
    std::ifstream file{ filename, std::ios::binary };
    if (!file.is_open()) {
      throw std::runtime_error(std::format("Failed to open file {}", filename));
    }

    // Read the file into a string
    const std::string contents((std::istreambuf_iterator<char>(file)),
      std::istreambuf_iterator<char>());

    // Convert to wstring (assuming UTF-8 input)
    // First, determine the required length
    size_t required = 0;
    if (mbstowcs_s(&required, nullptr, 0, contents.c_str(), 0) != 0) {
      throw std::runtime_error("Failed to convert to wide string");
    }

    std::wstring result(required, L'\0');
    if (mbstowcs_s(nullptr, result.data(), required, contents.c_str(), required - 1) != 0) {
      throw std::runtime_error("Failed to convert to wide string");
    }

    // Remove the trailing null character added by mbstowcs_s
    if (!result.empty() && result.back() == L'\0') {
      result.pop_back();
    }
    return result;
  }
}

auto main(const int argc, const char* argv[]) noexcept -> int {
  try {
    const auto contents = read_to_wstring(R"(D:\Projects\clox\test.lox)");

    std::wprintf(L"File contents: %ws", contents.data());

    constexpr auto my_token =
      lox::syntax::Token{ .kind = lox::syntax::TokenKind::identifier,
                         .lexeme = std::string_view{"main"},
        .span = lox::syntax::Span{.start = 0, .end = 4}

    };

    std::println("Token kind: {}, lexeme: {}",
      lox::syntax::token_kind_to_string(my_token.kind),
      my_token.lexeme);
  }
  catch (const std::exception& e) {
    std::println("Error: {}", e.what());
    return EXIT_FAILURE;
  }
}