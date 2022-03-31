//
// Created by andy on 2022-01-20.
//

#ifndef EVEMU_INVTYPE_H
#define EVEMU_INVTYPE_H

#include <string>
#include <cstdint>
#include <odb/core.hxx>

#pragma db model version(1, 1)

#pragma db object table("invTypes")
class InvType {
public:
    InvType() = default;

    uint16_t typeID() const {
        return typeID_;
    };
    void typeID(const uint16_t id_) {
        typeID_ = id_;
    };

    uint16_t groupID() const {
        return groupID_;
    };
    void groupID(const uint16_t id_) {
        groupID_ = id_;
    };

    const std::string& typeName() const {
        return typeName_;
    };
    void typeName(const std::string& name_) {
        typeName_ = name_;
    };

    const std::string& description() const {
        return description_;
    };
    void description(const std::string& description_v) {
        description_ = description_v;
    };

    uint16_t graphicID() const {
        return graphicID_;
    };
    void graphicID(const uint16_t id_) {
        graphicID_ = id_;
    };

    float radius() const {
        return radius_;
    };
    void radius(const float radius_v) {
        radius_ = radius_v;
    };

    float mass() const {
        return mass_;
    };
    void mass(const float mass_v) {
        mass_ = mass_v;
    };

    float volume() const {
        return volume_;
    };
    void volume(const float volume_v) {
        volume_ = volume_v;
    };

    float capacity() const {
        return capacity_;
    };
    void capacity(const float capacity_v) {
        capacity_ = capacity_v;
    };

    uint16_t partitionSize() const {
        return partitionSize_;
    };
    void partitionSize(const uint16_t size_) {
        partitionSize_ = size_;
    };

    uint8_t  raceID() const {
        return raceID_;
    };
    void raceID(const uint8_t  raceID_v) {
        raceID_ = raceID_v;
    };

    float basePrice() const {
        return basePrice_;
    };
    void basePrice(const float basePrice_v) {
        basePrice_ = basePrice_v;
    };

    bool published() const {
        return published_;
    };
    void published(const bool published_v) {
        published_ = published_v;
    };

    uint16_t marketGroupID() const {
        return marketGroupID_;
    };
    void marketGroupID(const uint16_t marketGroupID_v) {
        marketGroupID_ = marketGroupID_v;
    };

    float chanceOfDuplicating() const {
        return chanceOfDuplicating_;
    };
    void chanceOfDuplicating(const float chanceOfDuplicating_v) {
        chanceOfDuplicating_ = chanceOfDuplicating_v;
    };

    uint16_t copyTypeID() const {
        return copyTypeID_;
    };
    void copyTypeID(const uint16_t copyTypeID_v) {
        copyTypeID_ = copyTypeID_v;
    };

    uint16_t soundID() const {
        return soundID_;
    };
    void soundID(const uint16_t soundID_v) {
        soundID_ = soundID_v;
    };

    uint16_t iconID() const {
        return iconID_;
    };
    void iconID(const uint16_t iconID_v) {
        iconID_ = iconID_v;
    };

    uint16_t dataID() const {
        return dataID_;
    };
    void dataID(const uint16_t dataID_v) {
        dataID_ = dataID_v;
    };

    uint16_t typeNameID() const {
        return typeNameID_;
    };
    void typeNameID(const uint16_t typeNameID_v) {
        typeNameID_ = typeNameID_v;
    };

    uint16_t descriptionID() const {
        return descriptionID_;
    };
    void descriptionID(const uint16_t descriptionID_v) {
        descriptionID_ = descriptionID_v;
    };

private:
    friend class odb::access;

#pragma db id
#pragma db not_null
#pragma db access(typeID)
#pragma db column("typeID")
    uint16_t typeID_;

#pragma db default(0)
#pragma db not_null
#pragma db index
#pragma db access(groupID)
#pragma db column("groupID")
    uint16_t groupID_;

#pragma db access(typeName)
#pragma db column("typeName")
    std::string typeName_;

#pragma db access(description)
#pragma db column("description")
    std::string description_;

#pragma db index
#pragma db access(graphicID)
#pragma db column("graphicID")
    uint16_t graphicID_;

#pragma db not_null
#pragma db default(0)
#pragma db access(radius)
#pragma db column("radius")
    float radius_;

#pragma db not_null
#pragma db default(0)
#pragma db access(mass)
#pragma db column("mass")
    float mass_;

#pragma db not_null
#pragma db default(0)
#pragma db access(volume)
#pragma db column("volume")
    float volume_;

#pragma db not_null
#pragma db default(0)
#pragma db access(capacity)
#pragma db column("capacity")
    float capacity_;

#pragma db access(partitionSize)
#pragma db column("partitionSize")
    uint16_t partitionSize_;

#pragma db not_null
#pragma db default(0)
#pragma db index
#pragma db access(raceID)
#pragma db column("raceID")
    uint8_t raceID_;

#pragma db not_null
#pragma db default(0)
#pragma db access(basePrice)
#pragma db column("basePrice")
    float basePrice_;

#pragma db not_null
#pragma db default(false)
#pragma db access(published)
#pragma db column("published")
    bool published_;

#pragma db index
#pragma db access(marketGroupID)
#pragma db column("marketGroupID")
    uint16_t marketGroupID_;

#pragma db not_null
#pragma db default(0)
#pragma db access(chanceOfDuplicating)
#pragma db column("chanceOfDuplicating")
    float chanceOfDuplicating_;

#pragma db not_null
#pragma db default(0)
#pragma db access(copyTypeID)
#pragma db column("copyTypeID")
    uint16_t copyTypeID_;

#pragma db index
#pragma db access(soundID)
#pragma db column("soundID")
    uint16_t soundID_;

#pragma db index
#pragma db access(iconID)
#pragma db column("iconID")
    uint16_t iconID_;

#pragma db index
#pragma db access(dataID)
#pragma db column("dataID")
    uint16_t dataID_;

#pragma db index
#pragma db access(typeNameID)
#pragma db column("typeNameID")
    uint16_t typeNameID_;

#pragma db index
#pragma db access(descriptionID)
#pragma db column("descriptionID")
    uint16_t descriptionID_;
};

#pragma db view object(InvType)
struct InvType_BasePrice {
#pragma db column(InvType::typeID_)
    uint16_t typeID;
#pragma db column(InvType::basePrice_)
    float basePrice;
};

#endif //EVEMU_INVTYPE_H
