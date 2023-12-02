#pragma once

#include "ComponentClass.hpp"


ECS::ControllerButtonQuery IsDpadCenterPressed;
ECS::ControllerButtonQuery IsDpadUpPressed;
ECS::ControllerButtonQuery IsDpadUpRightPressed;
ECS::ControllerButtonQuery IsDpadRightPressed;
ECS::ControllerButtonQuery IsDpadDownRightPressed;
ECS::ControllerButtonQuery IsDpadDownPressed;
ECS::ControllerButtonQuery IsDpadDownLeftPressed;
ECS::ControllerButtonQuery IsDpadLeftPressed;
ECS::ControllerButtonQuery IsDpadUpLeftPressed;
ECS::ControllerButtonQuery IsLeftStickPressed;
ECS::ControllerButtonQuery IsRightStickPressed;
ECS::ControllerButtonQuery IsL1Pressed;
ECS::ControllerButtonQuery IsR1Pressed;
ECS::ControllerButtonQuery IsL2Pressed;
ECS::ControllerButtonQuery IsR2Pressed;
ECS::ControllerButtonQuery IsMenuPressed;
ECS::ControllerButtonQuery IsOptionsPressed;
ECS::ControllerButtonQuery IsCenterPadPressed;
ECS::ControllerButtonQuery IsPowerPressed;
ECS::ControllerButtonQuery IsMutePressed;
ECS::ControllerButtonQuery IsBotButtonPressed;
ECS::ControllerButtonQuery IsTopButtonPressed;
ECS::ControllerButtonQuery IsLeftButtonPressed;
ECS::ControllerButtonQuery IsRightButtonPressed;


typedef Float2 ControllerStickVectorQuery( const ECS::SingletonControllerInputComponent *component );

ControllerStickVectorQuery LeftStickQuery;
ControllerStickVectorQuery RightStickQuery;

typedef float ControllerStickAxisQuery( const ECS::SingletonControllerInputComponent *component );

ControllerStickAxisQuery LeftStickXAxisQuery;
ControllerStickAxisQuery LeftStickYAxisQuery;
ControllerStickAxisQuery RightStickXAxisQuery;
ControllerStickAxisQuery RightStickYAxisQuery;

typedef float ControllerTriggerQuery( const ECS::SingletonControllerInputComponent *component );

ControllerTriggerQuery LeftTriggerQuery;
ControllerTriggerQuery RightTriggerQuery;

void InitializeControllerButtonUtilities( ECS::SingletonControllerInputComponent *component );
