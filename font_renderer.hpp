#ifndef FONT_RENDERER_HPP_INCLUDED
#define FONT_RENDERER_HPP_INCLUDED
#include <SFML/Graphics/Text.hpp>

void render_font(sf::RenderWindow& win, const std::string& to_render, vec2f pos, vec4f col)
{
    static sf::Font font;
    static bool loaded = false;

    if(!loaded)
    {
        font.loadFromFile("VeraMono.ttf");
        loaded = true;
    }

    sf::Text txt;
    txt.setFont(font);
    txt.setString(to_render.c_str());
    txt.setCharacterSize(24);

    col = col * 255;

    txt.setColor(sf::Color(col.x(), col.y(), col.z(), col.w()));

    txt.setOrigin(txt.getLocalBounds().width/2.f, txt.getLocalBounds().height/2.f);

    win.draw(txt);
}

#endif // FONT_RENDERER_HPP_INCLUDED
