/***************************************************************************
 *            serialisation.hpp
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

#ifndef OPERA_SERIALISATION_HPP
#define OPERA_SERIALISATION_HPP

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/ostreamwrapper.h>
#include <filesystem>
#include <fstream>
#include "body.hpp"
#include "message.hpp"
#include "macros.hpp"

namespace Opera {

using FilePath = std::filesystem::path;

//! \brief Interface for serialisation into a file or a string
template<class T> class SerialiserInterface {
  public:
    //! \brief Serialise into file
    virtual void to_file(FilePath const& file) const = 0;
    //! \brief Serialise into String
    virtual std::string to_string() const = 0;
};

//! \brief Base implementation of serialisation, from a rapidjson Document
template<class T> class SerialiserBase : public SerialiserInterface<T> {
  public:
    //! \brief Pass the object by const reference
    SerialiserBase(T const& o) : obj(o) { }

    //! \brief Convert the object into a rapidjson document
    virtual rapidjson::Document to_document() const = 0;

  public:
    void to_file(FilePath const& file) const override {
        auto document = to_document();
        std::ofstream ofs(file);
        OPERA_ASSERT_MSG(ofs.is_open(), "Could not open file '" << file << "' for writing.")
        rapidjson::OStreamWrapper osw(ofs);
        rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
        document.Accept(writer);
    }

    std::string to_string() const override {
        auto document = to_document();
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        document.Accept(writer);
        return buffer.GetString();
    }

  protected:
    T const& obj;
};

template<class T> class Serialiser;

//! \brief Utility for making a JSON description from a presentation message
template<> class Serialiser<BodyPresentationMessage> : public SerialiserBase<BodyPresentationMessage> {
  public:
    using SerialiserBase::SerialiserBase;
  protected:
    rapidjson::Document to_document() const override;
};

//! \brief Utility for making a JSON description from a state message
template<> class Serialiser<BodyStateMessage> : public SerialiserBase<BodyStateMessage> {
  public:
    using SerialiserBase::SerialiserBase;
  protected:
    rapidjson::Document to_document() const override;
};

//! \brief Utility for making a JSON description from a notification message
template<> class Serialiser<CollisionNotificationMessage> : public SerialiserBase<CollisionNotificationMessage> {
  public:
    using SerialiserBase::SerialiserBase;
  protected:
    rapidjson::Document to_document() const override;
};

}

#endif //OPERA_SERIALISATION_HPP
