#ifndef FONT_RENDERER_HPP_INCLUDED
#define FONT_RENDERER_HPP_INCLUDED
#include <SFML/Graphics/Text.hpp>

void render_font(sf::RenderWindow& win, const std::string& to_render, vec2f pos, vec4f col, float scale = 1)
{
    static sf::Font font;
    static bool loaded = false;

    if(!loaded)
    {
        font.loadFromFile("WhiteRabbit.ttf");
        loaded = true;
    }

    sf::Text txt;
    txt.setFont(font);
    txt.setString(to_render.c_str());
    txt.setCharacterSize(round(16 * scale));
    //txt.setScale(0.5f, 0.5f);

    pos = round(pos);

    txt.setPosition(pos.x(), pos.y());

    col = col * 255;

    txt.setFillColor(sf::Color(col.x(), col.y(), col.z(), col.w()));

    txt.setOrigin(round(txt.getLocalBounds().width/2.f), 0.f);

    win.draw(txt);
}

#endif // FONT_RENDERER_HPP_INCLUDED
