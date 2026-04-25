#include "Symbol.h"

Symbol::Symbol(SymbolType type) : m_type(type) {}

SymbolType Symbol::type() const { return m_type; }

QString Symbol::name() const {
    switch (m_type) {
        case SymbolType::Cherry:     return QStringLiteral("Cherry");
        case SymbolType::Lemon:      return QStringLiteral("Lemon");
        case SymbolType::Orange:     return QStringLiteral("Orange");
        case SymbolType::Watermelon: return QStringLiteral("Melon");
        case SymbolType::Grape:      return QStringLiteral("Grape");
        case SymbolType::Strawberry: return QStringLiteral("Berry");
        case SymbolType::Bell:       return QStringLiteral("Bell");
        case SymbolType::Bar:        return QStringLiteral("BAR");
        case SymbolType::Bar2:       return QStringLiteral("BAR\nBAR");
        case SymbolType::Bar3:       return QStringLiteral("BAR\nBAR\nBAR");
        case SymbolType::Seven:      return QStringLiteral("7");
        case SymbolType::Wild:       return QStringLiteral("WILD");
        default:                     return QStringLiteral("?");
    }
}

QString Symbol::emoji() const {
    switch (m_type) {
        case SymbolType::Cherry:     return QStringLiteral("🍒");
        case SymbolType::Lemon:      return QStringLiteral("🍋");
        case SymbolType::Orange:     return QStringLiteral("🍊");
        case SymbolType::Watermelon: return QStringLiteral("🍉");
        case SymbolType::Grape:      return QStringLiteral("🍇");
        case SymbolType::Strawberry: return QStringLiteral("🍓");
        case SymbolType::Bell:       return QStringLiteral("🔔");
        case SymbolType::Bar:        return QStringLiteral("BAR");
        case SymbolType::Bar2:       return QStringLiteral("BAR\nBAR");
        case SymbolType::Bar3:       return QStringLiteral("BAR\nBAR\nBAR");
        case SymbolType::Seven:      return QStringLiteral("7");
        case SymbolType::Wild:       return QStringLiteral("★");
        default:                     return QStringLiteral("?");
    }
}

int Symbol::value() const {
    switch (m_type) {
        case SymbolType::Cherry:     return  2;
        case SymbolType::Lemon:      return  3;
        case SymbolType::Orange:     return  4;
        case SymbolType::Watermelon: return  5;
        case SymbolType::Grape:      return  6;
        case SymbolType::Strawberry: return  7;
        case SymbolType::Bell:       return 12;
        case SymbolType::Bar:        return 20;
        case SymbolType::Bar2:       return 35;
        case SymbolType::Bar3:       return 60;
        case SymbolType::Seven:      return 100;
        case SymbolType::Wild:       return 200;
        default:                     return  0;
    }
}

// Text-based symbols need smaller font rendering
bool Symbol::isText() const {
    switch (m_type) {
        case SymbolType::Bar:
        case SymbolType::Bar2:
        case SymbolType::Bar3:
        case SymbolType::Seven:
        case SymbolType::Wild:
            return true;
        default:
            return false;
    }
}

bool Symbol::matches(const Symbol& other) const {
    return m_type == SymbolType::Wild
        || other.m_type == SymbolType::Wild
        || m_type == other.m_type;
}
bool Symbol::operator==(const Symbol& other) const { return matches(other); }
bool Symbol::operator!=(const Symbol& other) const { return !matches(other); }
