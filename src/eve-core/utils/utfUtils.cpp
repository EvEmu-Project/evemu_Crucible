/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:     eve-moo
 */

#include "utfUtils.h"

/**
 * Convert a 32 bit character code to a UTF8 string
 * @param code The UTF32 character.
 * @return The UTF8 string. Returns an empty string on failure.
 */
std::string codeToUTF8(char32_t code)
{
    std::string out;
    if (code < 0x80)
    {
        out += (char) code;
        return out;
    }
    if (code < 0x800)
    {
        out += (char) (0xC0 + ((code >> 6) & 0x1f));
        out += (char) (0x80 + (code & 0x3f));
        return out;
    }
    if (code < 0x10000)
    {
        out += (char) (0xE0 + ((code >> 12) & 0x1f));
        out += (char) (0x80 + ((code >> 6) & 0x3f));
        out += (char) (0x80 + (code & 0x3f));
        return out;
    }
    if (code < 0x200000)
    {
        out += (char) (0xF0 + ((code >> 18) & 0x1f));
        out += (char) (0x80 + ((code >> 12) & 0x3f));
        out += (char) (0x80 + ((code >> 6) & 0x3f));
        out += (char) (0x80 + (code & 0x3f));
        return out;
    }
    if (code < 0x4000000)
    {
        out += (char) (0xF8 + ((code >> 24) & 0x1f));
        out += (char) (0x80 + ((code >> 18) & 0x3f));
        out += (char) (0x80 + ((code >> 12) & 0x3f));
        out += (char) (0x80 + ((code >> 6) & 0x3f));
        out += (char) (0x80 + (code & 0x3f));
        return out;
    }
    out += (char) (0xFC + ((code >> 30) & 0x1f));
    out += (char) (0x80 + ((code >> 24) & 0x3f));
    out += (char) (0x80 + ((code >> 18) & 0x3f));
    out += (char) (0x80 + ((code >> 12) & 0x3f));
    out += (char) (0x80 + ((code >> 6) & 0x3f));
    out += (char) (0x80 + (code & 0x3f));
    return out;
}

/**
 * Convert a 32 bit character code to a UTF16 string
 * @param code The UTF32 character.
 * @return The UTF16 string. Returns an empty string on failure.
 */
std::u16string codeToUTF16(char32_t code)
{
    std::u16string out;
    if (code < 0x010000)
    {
        out += (char16_t) code;
        return out;
    }
    char32_t ch = code - 0x010000;
    char32_t cl = ch & 0x03FF;
    ch >>= 10;
    out += (0xD800 + ch);
    out += (0xDC00 + cl);
    return out;
}

class UTFCodeParser
{
public:

    UTFCodeParser() { };

    UTFCodeParser(const UTFCodeParser& orig) { };

    virtual ~UTFCodeParser() { };

    /**
     * Add a UTF8 character to the code.
     * @param c The UTF8 character.
     * @return number of additional characters needed. 0 = complete, -1 on Error,
     */
    int addChar(char c)
    {
        if (byte == 0)
        {
            // First byte of sequence.
            if ((c & 0x80) == 0)
            {
                // Single byte code.
                code = c;
                return byte;
            }
            // Find number of bytes in code.
            while (c & 0x80)
            {
                c <<= 1;
                byte++;
            }
            code = c >> byte;
            byte--;
            if (byte == 0)
            {
                // Error: in middle of character.
                code = 0;
                return -1;
            }
            return byte;
        }
        // Continuing character.
        if ((c & 0xC0) != 0x80)
        {
            // Error: continuation code expected and not found.
            code = 0;
            return -1;
        }
        byte--;
        code <<= 6;
        code += c & 0x3f;
        return byte;
    }

    /**
     * Add a UTF16 character to the code.
     * @param c The UTF16 character.
     * @return number of additional characters needed. 0 = complete, -1 on Error,
     */
    int addChar(char16_t c)
    {
        if (byte == 0)
        {
            if (c < 0xD800 || c >= 0xE000)
            {
                code = c;
                return byte;
            }
            if (c >= 0xDC00)
            {
                // Error! This is a continuation character.
                code = 0;
                return -1;
            }
            byte = 1;
            code = c & 0x3FF;
            return byte;
        }
        if (c < 0xDC00 || c >= 0xE000)
        {
            // Error: Expected continuation character not found.
            if (c < 0xD800)
            {
                // Assume, good code following bad character.
                code = c;
                byte = 0;
                return 0;
            }
            // Assume, good start code following bad character.
            byte = 1;
            code = c & 0x3FF;
            return byte;
        }
        byte--;
        code <<= 10;
        code += c & 0x3FF;
        code += 0x010000;
        return byte;
    }

    /**
     * Add a UTF32 character to the code.
     * @param c The UTF32 character.
     * @return number of additional characters needed. 0 = complete, -1 on Error,
     */
    int addChar(char32_t c)
    {
        code = c;
        byte = 0;
        return byte;
    }

    /**
     * Get the current code as a UTF8 character string.
     * @return The UTF8 representation of the code of an empty string if the code is not valid.
     */
    std::string getUTF8()
    {
        if (byte == 0)
        {
            return codeToUTF8(code);
        }
        return "";
    }

    /**
     * Get the current code as a UTF16 character string.
     * @return The UTF16 representation of the code of an empty string if the code is not valid.
     */
    std::u16string getUTF16()
    {
        if (byte == 0)
        {
            return codeToUTF16(code);
        }
        return u"";
    }

    /**
     * Get the current code as a UTF32 character string.
     * @return The UTF32 representation of the code of an empty string if the code is not valid.
     */
    std::u32string getUTF32()
    {
        if (byte == 0)
        {
            std::u32string out;
            out += code;
            return out;
        }
        return U"";
    }

    /**
     * Reset the state.
     */
    void reset()
    {
        code = 0;
        byte = 0;
    }

    /**
     * Get the current character code value.
     * @return 
     */
    char32_t getCode()
    {
        return code;
    }

    /**
     * Is this current state a valid code?
     * @return True if code is valid.
     */
    bool valid()
    {
        return byte == 0;
    }

private:
    char32_t code = 0;
    int byte = 0;

};


std::u16string utf8to16(std::string &str)
{
    std::u16string out;
    UTFCodeParser code;
    for (char16_t c : str)
    {
        int res = code.addChar(c);
        if (res == -1)
        {
            // There was an error.
            return u"";
        }
        if (res == 0)
        {
            out += code.getUTF16();
        }
    }
    return out;
}

std::string utf16to8(std::u16string &str)
{
    std::string out;
    UTFCodeParser code;
    for (char c : str)
    {
        int res = code.addChar(c);
        if (res == -1)
        {
            // There was an error.
            return "";
        }
        if (res == 0)
        {
            out += code.getUTF8();
        }
    }
    return out;
}



