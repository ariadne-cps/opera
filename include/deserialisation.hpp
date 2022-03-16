/***************************************************************************
 *            deserialisation.hpp
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

#ifndef OPERA_DESERIALISATION_HPP
#define OPERA_DESERIALISATION_HPP

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/istreamwrapper.h>
#include <filesystem>
#include <fstream>
#include "body.hpp"
#include "message.hpp"
#include "macros.hpp"

namespace Opera {

using FilePath = std::filesystem::path;

//! \brief Base for a class deserialising a JSON file or string
template<class T> class DeserialiserBase {
  public:
    DeserialiserBase(FilePath const& file) {
        std::ifstream ifs(file);
        OPERA_ASSERT_MSG(ifs.is_open(), "Could not open '" << file << "' file for reading.")
        rapidjson::IStreamWrapper isw(ifs);
        _document.ParseStream(isw);
        OPERA_ASSERT_MSG(not _document.HasParseError(),"Parse error '" << _document.GetParseError() << "' at offset " << _document.GetErrorOffset())
    }

    DeserialiserBase(const char* text) {
        _document.Parse(text);
        OPERA_ASSERT_MSG(not _document.HasParseError(),"Parse error '" << _document.GetParseError() << "' at offset " << _document.GetErrorOffset())
    }

    //! \brief Convert to string
    std::string to_string() const {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        _document.Accept(writer);
        return buffer.GetString();
    }

    //! \brief Make the object
    virtual T make() const = 0;

    //! \brief Print to the standard output
    friend std::ostream& operator<<(std::ostream& os, DeserialiserBase const& d) {
        return os << d.to_string();
    }
  protected:
    rapidjson::Document _document;
};

template<class T> class Deserialiser;

//! \brief Converter to a BodyPresentationMessage from a JSON description file
template<> class Deserialiser<BodyPresentationMessage> : public DeserialiserBase<BodyPresentationMessage> {
  public:
    using DeserialiserBase::DeserialiserBase;
    BodyPresentationMessage make() const override;
};

//! \brief Converter to a BodyStateMessage from a JSON description file
template<> class Deserialiser<BodyStateMessage> : public DeserialiserBase<BodyStateMessage> {
  public:
    using DeserialiserBase::DeserialiserBase;
    BodyStateMessage make() const override;
};

//! \brief Convert to a CollisionNotificationMessage from a JSON description file
template<> class Deserialiser<CollisionNotificationMessage> : public DeserialiserBase<CollisionNotificationMessage> {
  public:
    using DeserialiserBase::DeserialiserBase;
    CollisionNotificationMessage make() const override;
};

}

#endif //OPERA_DESERIALISATION_HPP
