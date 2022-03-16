/***************************************************************************
 *            command_line_interface.cpp
 *
 *  Copyright  2021  Luca Geretti
 *
 ****************************************************************************/

/*
 * This file is part of Opera, under the MIT license.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "conclog/include/logging.hpp"
#include "command_line_interface.hpp"

using namespace ConcLog;

namespace Opera {

ArgumentStream::ArgumentStream(List<std::string> const& args) {
    OPERA_PRECONDITION(args.size() > 0)
    for (auto arg : args) _args.push(arg);
}

std::string ArgumentStream::peek() const {
    OPERA_PRECONDITION(not empty())
    return _args.front();
}

std::string ArgumentStream::pop() {
    auto val = peek();
    _args.pop();
    return val;
}

bool ArgumentStream::empty() const {
    return _args.empty();
}

SizeType ArgumentStream::size() const {
    return _args.size();
}

ArgumentPack::ArgumentPack(std::string const& id, VoidFunction const& processor) : _id(id), _processor(processor) { }

std::string const& ArgumentPack::id() const {
    return _id;
}

void ArgumentPack::process() const {
    _processor();
}

bool ArgumentPack::operator<(ArgumentPack const& other) const {
    return this->_id < other._id;
}

//! \brief Supplies base behavior for argument parsers
class ArgumentParserBase : public ArgumentParserInterface {
  protected:
    ArgumentParserBase(std::string const& short_id, std::string const& long_id, std::string const& instructions)
        : _short_id(short_id), _long_id(long_id), _instructions(instructions) { }
  public:
    std::string short_id() const { return _short_id; }
    std::string long_id() const { return _long_id; }
    bool has_short_id() const { return not _short_id.empty(); }
    std::string instructions() const { return _instructions; }

    ArgumentPack consume(ArgumentStream& stream) const override;

    //! \brief If a value is required for this argument
    virtual bool requires_value() const = 0;

    bool is_consumable(ArgumentStream const& stream) const override;
    SizeType help_description_header_size() const override;
    std::string help_description(SizeType num_chars_to_align_instructions) const override;

    virtual ~ArgumentParserBase() = default;

  protected:
    virtual VoidFunction create_processor(ArgumentStream& stream) const = 0;

  private:
    std::string _short_id;
    std::string _long_id;
    std::string _instructions;
};

SizeType ArgumentParserBase::help_description_header_size() const {
    SizeType result = 4+long_id().size();
    if (has_short_id()) result+= 4+short_id().size();
    if (requires_value()) result += 8;
    return result;
}

std::string ArgumentParserBase::help_description(SizeType num_chars_to_separate_instructions) const {
    StringStream ss;
    ss << "[";
    if (has_short_id()) { ss << "-" << short_id() << " | "; }
    ss << "--" << long_id() << "]";
    if (requires_value()) { ss << " <value>"; }
    ss << std::string(num_chars_to_separate_instructions,' ') << instructions();
    return ss.str();
}

bool ArgumentParserBase::is_consumable(const ArgumentStream &stream) const {
    auto arg = stream.peek();
    if (has_short_id()) {
        std::string short_argument = "-"+short_id();
        if (arg == short_argument) return true;
    }
    std::string long_argument = "--"+long_id();
    return (arg == long_argument);
}

ArgumentPack ArgumentParserBase::consume(ArgumentStream& stream) const {
    stream.pop(); // Pop out the identifier already recognised as the one for this parser
    return ArgumentPack(_long_id,create_processor(stream));
}

class ValuedArgumentParserBase : public ArgumentParserBase {
  public:
    ValuedArgumentParserBase(std::string const& s, std::string const& l, std::string const& i) : ArgumentParserBase(s,l,i) { }
    bool requires_value() const override { return true; }
    VoidFunction create_processor(ArgumentStream& stream) const override final {
        VoidFunction f;
        if (stream.empty()) { throw MissingArgumentValueException(long_id()); }
        try {
            f = _create_processor(stream);
        } catch (std::exception&) {
            throw InvalidArgumentValueException(long_id());
        }
        return f;
    }
  protected:
    virtual VoidFunction _create_processor(ArgumentStream& stream) const = 0;
};

class UnvaluedArgumentParserBase : public ArgumentParserBase {
  public:
    UnvaluedArgumentParserBase(std::string const& s, std::string const& l, std::string const& i) : ArgumentParserBase(s,l,i) { }
    bool requires_value() const override { return false; }
};

class HelpArgumentParser : public UnvaluedArgumentParserBase {
  public:
    HelpArgumentParser() : UnvaluedArgumentParserBase(
            "h","help","Show this list of supported arguments") { }

    VoidFunction create_processor(ArgumentStream&) const override {
        return []{};
    }
};

class SchedulerArgumentParser : public ValuedArgumentParserBase {
public:
    SchedulerArgumentParser() : ValuedArgumentParserBase(
            "s","scheduler","Choose the logging scheduler as a <value> in [ immediate | blocking | nonblocking ] (default: nonblocking)") { }

    VoidFunction _create_processor(ArgumentStream& stream) const override {
        std::string val = stream.pop();
        if (val == "immediate") return []{ Logger::instance().use_immediate_scheduler(); };
        else if (val == "blocking") return []{ Logger::instance().use_blocking_scheduler(); };
        else if (val == "nonblocking") return []{ Logger::instance().use_nonblocking_scheduler(); };
        else throw std::exception();
    }
};

class ThemeArgumentParser : public ValuedArgumentParserBase {
  public:
    ThemeArgumentParser() : ValuedArgumentParserBase(
            "t","theme","Choose the logging theme as a <value> in [ none | light | dark ] (default: none)") { }

    VoidFunction _create_processor(ArgumentStream& stream) const override {
        TerminalTextTheme theme = TT_THEME_NONE;
        std::string val = stream.pop();
        if (val != "none") {
            if (val == "light") theme = TT_THEME_LIGHT;
            else if (val == "dark") theme = TT_THEME_DARK;
            else throw std::exception();
        }
        return [theme]{ Logger::instance().configuration().set_theme(theme); };
    }
};

class VerbosityArgumentParser : public ValuedArgumentParserBase {
  public:
    VerbosityArgumentParser() : ValuedArgumentParserBase(
            "v","verbosity","Choose the logging verbosity as a non-negative integer <value> (default: 0)") { }

    VoidFunction _create_processor(ArgumentStream& stream) const override {
        int val = std::stoi(stream.pop());
        if (val < 0) throw std::exception();
        return [val]{ Logger::instance().configuration().set_verbosity(static_cast<unsigned int>(val)); };
    }
};

CommandLineInterface::CommandLineInterface() : _parsers({
    HelpArgumentParser(),SchedulerArgumentParser(),ThemeArgumentParser(),VerbosityArgumentParser()
    }) { }

bool CommandLineInterface::acquire(int argc, const char* argv[]) const {
    List<std::string> args;
    for (int i = 0; i < argc; ++i) args.push_back(std::string(argv[i]));
    return acquire(args);
}

bool CommandLineInterface::acquire(List<std::string> const& args) const {
    ArgumentStream stream(args);
    stream.pop(); // Pop out the function pointer or script name

    Set<ArgumentPack> packs;
    while (not stream.empty()) {
        bool consumed = false;
        for (auto& parser : _parsers) {
            if (parser.is_consumable(stream)) {
                consumed = true;
                try {
                    auto p = parser.consume(stream);
                    if (packs.contains(p)) {
                        std::clog << "Argument '" << p.id() << "' specified multiple times.\n\n"
                            << parser.help_description(4) << std::endl;
                        return false;
                    } else if (p.id() == "help") {
                        p.process(); _print_help();
                        return false;
                    } else if (p.id() == "scheduler") {
                        // Need to process this before the others
                        p.process();
                    } else packs.insert(p);
                } catch(MissingArgumentValueException& exc) {
                    std::clog << "A value is required by the '" << exc.what() << "' argument, but it is not supplied.\n\n"
                        << parser.help_description(4) << std::endl;
                    return false;
                } catch(InvalidArgumentValueException& exc) {
                    std::clog << "Invalid '" << exc.what() << "' argument value, see the usage below:" << "\n\n" << parser.help_description(4) << std::endl;
                    return false;
                }
                break;
            }
        }
        if (not consumed) {
            std::clog << "Unrecognised command-line argument '" << stream.peek() << "'" << std::endl << std::endl;
            _print_help();
            return false;
        }
    }

    for (auto p : packs) p.process();

    return true;
}

void CommandLineInterface::_print_help() const {

    List<SizeType> chars(_parsers.size(),0);
    SizeType max_chars = 0;
    for (SizeType i=0; i<_parsers.size(); ++i)  {
        chars[i] = _parsers[i].help_description_header_size();
        if (chars[i] > max_chars) max_chars = chars[i];
    }

    std::clog << "Supported arguments:" << std::endl;
    for (SizeType i=0; i<_parsers.size(); ++i) {
        std::clog << "    " << _parsers[i].help_description(4+max_chars-chars[i]) << std::endl;
    }
}

} // namespace Opera
