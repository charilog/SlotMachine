#pragma once
#include <QString>

// Real slot machine symbol set — ordered by increasing rarity/value
enum class SymbolType {
    Cherry   = 0,
    Lemon    = 1,
    Orange   = 2,
    Watermelon = 3,
    Grape    = 4,
    Strawberry = 5,
    Bell     = 6,
    Bar      = 7,   // single BAR
    Bar2     = 8,   // 2BAR
    Bar3     = 9,   // 3BAR
    Seven    = 10,
    Wild     = 11,
    COUNT    = 12
};

class Symbol {
public:
    explicit Symbol(SymbolType type = SymbolType::Cherry);

    SymbolType type()    const;
    QString    name()    const;
    QString    emoji()   const;   // emoji or short text
    int        value()   const;
    bool       isText()  const;   // true for BAR/2BAR/3BAR/7/WILD (drawn smaller)

    bool matches(const Symbol& other) const;
    bool operator==(const Symbol& other) const;
    bool operator!=(const Symbol& other) const;

private:
    SymbolType m_type;
};
