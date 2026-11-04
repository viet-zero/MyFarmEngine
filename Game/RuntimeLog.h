#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <streambuf>

namespace MyGame
{
    // Preserve stdout/stderr (including CTest pipes), and also persist launch diagnostics.
    class RuntimeLog
    {
        class Tee : public std::streambuf
        {
        public:
            Tee(std::streambuf* original,std::ofstream& file):m_Original(original),m_File(file) {}
        protected:
            std::streamsize xsputn(const char* text,std::streamsize count) override
            {
                if(m_Original) m_Original->sputn(text,count);
                m_File.write(text,count);
                return m_File ? count : 0;
            }
            int_type overflow(int_type value) override
            {
                if(traits_type::eq_int_type(value,traits_type::eof())) return traits_type::not_eof(value);
                const char c=traits_type::to_char_type(value);
                return xsputn(&c,1)==1 ? value : traits_type::eof();
            }
            int sync() override
            {
                if(m_Original) m_Original->pubsync();
                m_File.flush(); return m_File ? 0 : -1;
            }
        private:
            std::streambuf* m_Original;
            std::ofstream& m_File;
        };
    public:
        RuntimeLog();
        ~RuntimeLog();
        bool Ready() const { return m_File.is_open(); }
        const std::filesystem::path& Path() const { return m_Path; }
    private:
        std::filesystem::path m_Path;
        std::ofstream m_File;
        std::streambuf* m_Out=std::cout.rdbuf();
        std::streambuf* m_Error=std::cerr.rdbuf();
        Tee m_OutTee{m_Out,m_File},m_ErrorTee{m_Error,m_File};
    };
}
