/***************************************************************************
 *            deserialisation.cpp
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

#include "deserialisation.hpp"
#include "mode.hpp"

namespace Opera {

using namespace rapidjson;

BodyPresentationMessage Deserialiser<BodyPresentationMessage>::make() const {
    List<Pair<IdType,IdType>> point_ids;
    for (auto& extremes : _document["pointIds"].GetArray())
        point_ids.push_back(std::make_pair(extremes[0].GetUint(),extremes[1].GetUint()));

    List<FloatType> thicknesses;
    for (auto& thickness : _document["thicknesses"].GetArray())
        thicknesses.push_back(thickness.GetDouble());

    if (_document["isHuman"].GetBool())
        return BodyPresentationMessage(_document["id"].GetString(), point_ids, thicknesses);
    else
        return BodyPresentationMessage(_document["id"].GetString(), _document["messageFrequency"].GetUint64(), point_ids, thicknesses);
}

BodyStateMessage Deserialiser<BodyStateMessage>::make() const {
    Map<String,String> mode_values;
    if (_document.HasMember("mode"))
        for (auto& v : _document["mode"].GetObject())
            mode_values.insert(std::make_pair(v.name.GetString(),v.value.GetString()));
    List<List<Point>> points;
    for (auto& point_samples : _document["continuousState"].GetArray()) {
        List<Point> samples;
        for (auto& pt : point_samples.GetArray())
            samples.emplace_back(pt[0].GetDouble(),pt[1].GetDouble(),pt[2].GetDouble());
        points.push_back(samples);
    }

    return BodyStateMessage(_document["bodyId"].GetString(), Mode(mode_values), points, _document["timestamp"].GetUint64());
}

CollisionNotificationMessage Deserialiser<CollisionNotificationMessage>::make() const {
    Map<String,String> collision_mode_values;
    for (auto& v : _document["collisionMode"].GetObject())
        collision_mode_values.insert(std::make_pair(v.name.GetString(), v.value.GetString()));

    return CollisionNotificationMessage(_document["human"]["bodyId"].GetString(),
                                        _document["human"]["segmentId"].GetUint(),
                                        _document["robot"]["bodyId"].GetString(),
                                        _document["robot"]["segmentId"].GetUint(),
                                        _document["currentTime"].GetUint64(),
                                        Interval<TimestampType>(_document["collisionDistance"]["lower"].GetUint64(),
                                       _document["collisionDistance"]["upper"].GetUint64()),
                                        collision_mode_values,
                                        _document["likelihood"].GetDouble());
}

}
