/*
 * Copyright (C) 2025 Ték Róbert Máté <eppenpontaz@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <concepts>
#include <iostream>
#include <limits>
#include <string_view>

// Slightly over-the-top C++ template metaprogramming magic to define
// reusable command-line prompts with automatic predicate checking.
// Example usage can be seen in the C++ executables.
namespace TRM
{

    template<class T, auto Msg, auto Predicate>
    struct Part{};

    template<class>
    constexpr bool IsPart = false;

    template<class T, auto Msg, auto P>
    constexpr bool IsPart<Part<T, Msg, P>> = true;

    template<class>
    struct PartInfo{};

    template<class T, auto Msg, auto P>
    struct PartInfo<Part<T, Msg, P>>
    {
        using Type = T;
        static constexpr auto Message = Msg;
        inline static auto& Predicate = P;
    };

    template<class P>
    concept IsValidPart = requires ()
    {
        typename PartInfo<P>::Type;
        std::cout << PartInfo<P>::Message;
        requires std::predicate<decltype(PartInfo<P>::Predicate), typename PartInfo<P>::Type>;
    };

#define PROMPT_PART(Name, Type, Message, L) \
    constexpr const char Name ## _Prompt_Message [] = Message; \
    using Name = Part<Type, Name ## _Prompt_Message, L>; \
    static_assert(IsValidPart<Name>)

    template<class>
    struct PromptPreferences{};

    template<class T>
    concept HasPreferences = requires ()
    {
        typename PromptPreferences<T>::Parts;
        requires std::is_trivially_default_constructible_v<PromptPreferences<T>>;
    };

    template<class T>
    constexpr bool h = HasPreferences<T>;

#define PROMPT_PREFERENCES(Type, ...) \
    template<> struct TRM::PromptPreferences<Type> { using Parts = std::tuple<__VA_ARGS__>; }

    template<class T> requires (!HasPreferences<T>)
    T Prompt(const std::string_view message, std::predicate<T> auto&& predicate = [](const T&){ return true; })
    {
        using namespace std;

        cout << message;
        T t;
        while(!(cin >> t) || !predicate(t))
        {
            cout << "   That is not valid ! Try again: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        return t;
    }

    template<class T, class... Parts> requires (IsPart<Parts> && ...)
    T Prompt(std::tuple<Parts...>)
    {
        return T{ Prompt<typename PartInfo<Parts>::Type>(PartInfo<Parts>::Message, PartInfo<Parts>::Predicate)... };
    }

    template<HasPreferences T>
    T Prompt(const std::string_view message, std::predicate<T> auto&& predicate = [](const T&){ return true; })
    {
        using namespace std;

        cout << message << '\n';
        for(;;)
        {
            auto obj = Prompt<T>(typename PromptPreferences<T>::Parts{});
            if (predicate(obj))
                return obj;
            cout << "   That is not valid ! Try again.\n";
        }
    }

} // namespace TRM
