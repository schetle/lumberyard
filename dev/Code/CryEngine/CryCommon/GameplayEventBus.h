/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
#pragma once
#include <AzCore/EBus/EBus.h>
#include <AzCore/Component/EntityId.h>
#include <AzCore/std/any.h>
#include <AzCore/Component/ComponentApplicationBus.h>
#include <AzCore/Serialization/SerializeContext.h>

namespace AZ
{
    class GameplayNotificationId
    {
    public:
        AZ_TYPE_INFO(GameplayNotificationId, "{C5225D36-7068-412D-A46E-DDF79CA1D7FF}");
        GameplayNotificationId() = default;
        GameplayNotificationId(const AZ::EntityId& entityChannel, AZ::Crc32 actionNameCrc, const AZ::Uuid& payloadType)
            : m_channel(entityChannel)
            , m_actionNameCrc(actionNameCrc)
            , m_payloadTypeId(payloadType)
        { }
        GameplayNotificationId(const AZ::EntityId&  entityChannel, const char* actionName, const AZ::Uuid& payloadType)
            : GameplayNotificationId(entityChannel, AZ::Crc32(actionName), payloadType) {}

        //////////////////////////////////////////////////////////////////////////
        // Deprecated constructors.  These will be removed in 1.12
        GameplayNotificationId(const AZ::EntityId& entityChannel, AZ::Crc32 actionNameCrc)
            : GameplayNotificationId(entityChannel, actionNameCrc, AZ::Uuid::CreateNull())
        {
            AZ_Warning("GameplayNotificationId", false, "You are using a deprecated constructor.  You must now create the bus id with the type you are expecting to send/recieve");
        }
        GameplayNotificationId(const AZ::EntityId&  entityChannel, const char* actionName)
            : GameplayNotificationId(entityChannel, AZ::Crc32(actionName)) { }
        //////////////////////////////////////////////////////////////////////////

        bool operator==(const GameplayNotificationId& rhs) const { return m_channel == rhs.m_channel && m_actionNameCrc == rhs.m_actionNameCrc && m_payloadTypeId == rhs.m_payloadTypeId; }
        GameplayNotificationId Clone() const { return *this; }
        AZStd::string ToString() const
        {
            AZ::SerializeContext* serializeContext = nullptr;
            AZ::ComponentApplicationBus::BroadcastResult(serializeContext, &AZ::ComponentApplicationBus::Events::GetSerializeContext);
            auto&& classData = serializeContext->FindClassData(m_payloadTypeId);
            AZStd::string payloadType = classData ? classData->m_name : m_payloadTypeId.ToString<AZStd::string>();
            return AZStd::string::format("%llu, %lu, %s", static_cast<AZ::u64>(m_channel), static_cast<AZ::u32>(m_actionNameCrc), payloadType.c_str()); 
        }

        AZ::EntityId m_channel = AZ::EntityId(0);
        AZ::Crc32 m_actionNameCrc;
        AZ::Uuid m_payloadTypeId = AZ::Uuid::CreateNull();
    };


    //  //////////////////////////////////////////////////////////////////////////
    //  /// The Event Notification bus is used to alert gameplay systems that an event
    //  /// has occurred successfully or in a failure state.
    //  //////////////////////////////////////////////////////////////////////////
    class GameplayNotifications
        : public AZ::EBusTraits
    {
    public:
        static const EBusAddressPolicy AddressPolicy = EBusAddressPolicy::ById;
        typedef GameplayNotificationId BusIdType;
        virtual ~GameplayNotifications() = default;
        virtual void OnEventBegin(const AZStd::any&) {}
        virtual void OnEventUpdating(const AZStd::any&) {}
        virtual void OnEventEnd(const AZStd::any&) {}
    };
    using GameplayNotificationBus = AZ::EBus<GameplayNotifications>;
} // namespace AZ

namespace AZStd
{
    template <>
    struct hash < AZ::GameplayNotificationId >
    {
        inline size_t operator()(const AZ::GameplayNotificationId& actionId) const
        {
            AZStd::hash<AZ::EntityId> entityIdHasher;
            size_t retVal = entityIdHasher(actionId.m_channel);
            AZStd::hash_combine(retVal, actionId.m_actionNameCrc);
            AZStd::hash_combine(retVal, actionId.m_payloadTypeId);
            return retVal;
        }
    };
}