/***************************************************************************
 *            serialisation.cpp
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

#include "serialisation.hpp"
#include "mode.hpp"

namespace Opera {

using namespace rapidjson;

Document Serialiser<BodyPresentationMessage>::to_document() const {

    Document document;
    document.SetObject();
    Document::AllocatorType& allocator = document.GetAllocator();

    Value id;
    id.SetString(obj.id().c_str(),static_cast<rapidjson::SizeType>(obj.id().length()));
    document.AddMember("id",id,allocator);
    document.AddMember("isHuman",Value().SetBool(obj.is_human()),allocator);
    if (not obj.is_human()) document.AddMember("messageFrequency",Value().SetUint64(obj.message_frequency()),allocator);

    Value thicknesses;
    Value point_ids;
    thicknesses.SetArray();
    point_ids.SetArray();
    for (SizeType i=0; i<obj.point_ids().size(); ++i) {
        thicknesses.PushBack(Value().SetDouble(obj.thicknesses()[i]),allocator);
        Value points;
        points.SetArray();
        points.PushBack(Value().SetUint(obj.point_ids()[i].first),allocator);
        points.PushBack(Value().SetUint(obj.point_ids()[i].second),allocator);
        point_ids.PushBack(points,allocator);
    }
    document.AddMember("pointIds",point_ids,allocator);
    document.AddMember("thicknesses",thicknesses,allocator);

    return document;
}

Document Serialiser<BodyStateMessage>::to_document() const {

    Document document;
    document.SetObject();
    Document::AllocatorType& allocator = document.GetAllocator();

    Value id;
    id.SetString(obj.id().c_str(),static_cast<rapidjson::SizeType>(obj.id().length()));
    document.AddMember("bodyId",id,allocator);

    if (not obj.mode().is_empty()) {
        Value mode;
        mode.SetObject();
        for (auto const& v : obj.mode().values()) {
            mode.AddMember(Value().SetString(v.first.c_str(),static_cast<rapidjson::SizeType>(v.first.length()),allocator),
                                    Value().SetString(v.second.c_str(),static_cast<rapidjson::SizeType>(v.second.length()), allocator),allocator);
        }
        document.AddMember("mode",mode,allocator);
    }

    Value continuous_state;
    continuous_state.SetArray();
    for (auto const& samples : obj.points()) {
        Value samples_array;
        samples_array.SetArray();
        for (auto const& point : samples) {
            Value coordinates;
            coordinates.SetArray();
            coordinates.PushBack(Value().SetDouble(point.x),allocator)
            .PushBack(Value().SetDouble(point.y),allocator)
            .PushBack(Value().SetDouble(point.z),allocator);
            samples_array.PushBack(coordinates,allocator);
        }
        continuous_state.PushBack(samples_array,allocator);
    }
    document.AddMember("continuousState",continuous_state,allocator);
    document.AddMember("timestamp",Value().SetUint64(obj.timestamp()),allocator);

    return document;
}

Document Serialiser<CollisionNotificationMessage>::to_document() const {

    Document document;
    document.SetObject();
    Document::AllocatorType& allocator = document.GetAllocator();

    Value human;
    human.SetObject();
    human.AddMember("bodyId",Value().SetString(obj.human_id().c_str(),static_cast<rapidjson::SizeType>(obj.human_id().length()),allocator),allocator);
    human.AddMember("segmentId",Value().SetUint(obj.human_segment_id()),allocator);
    document.AddMember("human",human,allocator);

    Value robot;
    robot.SetObject();
    robot.AddMember("bodyId",Value().SetString(obj.robot_id().c_str(),static_cast<rapidjson::SizeType>(obj.robot_id().length()),allocator),allocator);
    robot.AddMember("segmentId",Value().SetUint(obj.robot_segment_id()),allocator);
    document.AddMember("robot",robot,allocator);

    document.AddMember("currentTime", Value().SetUint64(obj.current_time()), allocator);

    Value collision_distance;
    collision_distance.SetObject();
    collision_distance.AddMember("lower", Value().SetUint64(obj.collision_distance().lower()), allocator);
    collision_distance.AddMember("upper", Value().SetUint64(obj.collision_distance().upper()), allocator);
    document.AddMember("collisionDistance", collision_distance, allocator);

    if (not obj.collision_mode().is_empty()) {
        Value collisionMode;
        collisionMode.SetObject();
        for (auto const& v : obj.collision_mode().values()) {
            collisionMode.AddMember(Value().SetString(v.first.c_str(), static_cast<rapidjson::SizeType>(v.first.length()), allocator),
                                    Value().SetString(v.second.c_str(),static_cast<rapidjson::SizeType>(v.second.length()), allocator), allocator);
        }
        document.AddMember("collisionMode", collisionMode, allocator);
    }

    document.AddMember("likelihood",Value().SetDouble(obj.likelihood()),allocator);

    return document;
}

}
