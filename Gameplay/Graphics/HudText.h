#pragma once
#include "Graphics/Canvas2D.h"
#include <array>
#include <string_view>
namespace MyGame
{
    // Minimal ASCII labels for the prototype storage panel, no image/font assets.
    inline std::array<unsigned char,7> HudGlyph(char c)
    {
        switch(c)
        {
        case 'B':return {30,17,17,30,17,17,30};case 'E':return {31,16,16,30,16,16,31};
        case 'F':return {31,16,16,30,16,16,16};case 'L':return {16,16,16,16,16,16,31};
        case 'M':return {17,27,21,21,17,17,17};case 'P':return {30,17,17,30,16,16,16};
        case 'W':return {17,17,17,21,21,27,17};case 'Q':return {14,17,17,17,21,18,13};
        case 'A':return {14,17,17,31,17,17,17};case 'C':return {14,17,16,16,16,17,14};
        case 'D':return {30,17,17,17,17,17,30};case 'G':return {14,17,16,23,17,17,15};
        case 'H':return {17,17,17,31,17,17,17};case 'I':return {31,4,4,4,4,4,31};
        case 'K':return {17,18,20,24,20,18,17};case 'N':return {17,25,25,21,19,19,17};
        case 'O':return {14,17,17,17,17,17,14};case 'R':return {30,17,17,30,20,18,17};
        case 'S':return {15,16,16,14,1,1,30};case 'T':return {31,4,4,4,4,4,4};
        case 'U':return {17,17,17,17,17,17,14};case 'V':return {17,17,17,17,17,10,4};
        case 'Y':return {17,17,10,4,4,4,4};
        case '0':return {14,17,19,21,25,17,14};case '1':return {4,12,4,4,4,4,14};
        case '2':return {14,17,1,2,4,8,31};case '3':return {30,1,1,14,1,1,30};
        case '4':return {2,6,10,18,31,2,2};case '5':return {31,16,16,30,1,1,30};
        case '6':return {14,16,16,30,17,17,14};case '7':return {31,1,2,4,8,8,8};
        case '8':return {14,17,17,14,17,17,14};case '9':return {14,17,17,15,1,1,14};
        case '/':return {1,2,2,4,8,8,16};case '-':return {0,0,0,31,0,0,0};default:return {};
        }
    }
    inline void HudText(MyEngine::Canvas2D& canvas,std::string_view text,float x,float y,float scale,
                        std::array<float,4> color={0.15f,0.20f,0.12f,1})
    {
        for(char c:text)
        {
            const auto rows=HudGlyph(c);
            for(std::size_t row=0;row<rows.size();++row)
            {
                int column=0;
                while(column<5)
                {
                    if(!(rows[row] & (1u<<(4-column))))
                    {
                        ++column;
                        continue;
                    }
                    const int start=column++;
                    while(column<5 && (rows[row] & (1u<<(4-column)))) ++column;
                    canvas.Rectangle(x+start*scale,y+static_cast<float>(row)*scale,
                                     (column-start)*scale,scale,color);
                }
            }
            x+=6*scale;
        }
    }
}
