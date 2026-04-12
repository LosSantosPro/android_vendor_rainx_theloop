/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Minimal USB gadget HAL for theloop (mt6877 smart speaker).
 *
 * The stock vendor binary (android.hardware.usb.gadget-aidl-service.mediatekv1.0)
 * crashes with SIGABRT in its UsbGadget() constructor.  LineageOS 22.2's
 * UsbService.onBootPhase blocks the display thread on CompletableFuture.join()
 * waiting for IUsbGadget/default, which triggers a Watchdog kill of SystemServer
 * after 60 s.
 *
 * This stub registers IUsbGadget/default so UsbService unblocks.
 * It reads the real UDC speed from sysfs but otherwise does not touch
 * configfs or any other USB gadget hardware - the speaker has no user-facing
 * USB gadget functionality.
 *
 * Based on hardware/interfaces/usb/gadget/aidl/default/ (Apache 2.0).
 */

#define LOG_TAG "android.hardware.usb.gadget.aidl-service.theloop"

#include "UsbGadget.h"

namespace aidl {
namespace android {
namespace hardware {
namespace usb {
namespace gadget {

UsbGadget::UsbGadget()
    : mCurrentUsbFunctions(0),
      mCurrentUsbFunctionsApplied(false),
      mUsbSpeed(UsbSpeed::UNKNOWN) {
}

ScopedAStatus UsbGadget::getCurrentUsbFunctions(
        const shared_ptr<IUsbGadgetCallback> &callback,
        int64_t in_transactionId) {
    if (callback == nullptr)
        return ScopedAStatus::fromExceptionCode(EX_NULL_POINTER);

    ScopedAStatus ret = callback->getCurrentUsbFunctionsCb(
        mCurrentUsbFunctions,
        mCurrentUsbFunctionsApplied ? Status::FUNCTIONS_APPLIED
                                    : Status::FUNCTIONS_NOT_APPLIED,
        in_transactionId);
    if (!ret.isOk())
        ALOGE("Call to getCurrentUsbFunctionsCb failed %s",
              ret.getDescription().c_str());

    return ScopedAStatus::ok();
}

ScopedAStatus UsbGadget::getUsbSpeed(
        const shared_ptr<IUsbGadgetCallback> &callback,
        int64_t in_transactionId) {
    std::string current_speed;
    if (ReadFileToString(SPEED_PATH, &current_speed)) {
        current_speed = Trim(current_speed);
        ALOGI("current USB speed is %s", current_speed.c_str());
        if (current_speed == "low-speed")
            mUsbSpeed = UsbSpeed::LOWSPEED;
        else if (current_speed == "full-speed")
            mUsbSpeed = UsbSpeed::FULLSPEED;
        else if (current_speed == "high-speed")
            mUsbSpeed = UsbSpeed::HIGHSPEED;
        else if (current_speed == "super-speed")
            mUsbSpeed = UsbSpeed::SUPERSPEED;
        else if (current_speed == "super-speed-plus")
            mUsbSpeed = UsbSpeed::SUPERSPEED_10Gb;
        else
            mUsbSpeed = UsbSpeed::UNKNOWN;
    } else {
        ALOGI("Fail to read speed from %s, reporting UNKNOWN", SPEED_PATH);
        mUsbSpeed = UsbSpeed::UNKNOWN;
    }

    if (callback) {
        ScopedAStatus ret = callback->getUsbSpeedCb(mUsbSpeed, in_transactionId);
        if (!ret.isOk())
            ALOGE("Call to getUsbSpeedCb failed %s",
                  ret.getDescription().c_str());
    }

    return ScopedAStatus::ok();
}

ScopedAStatus UsbGadget::setCurrentUsbFunctions(
        int64_t functions,
        const shared_ptr<IUsbGadgetCallback> &callback,
        int64_t /* timeoutMs */,
        int64_t in_transactionId) {
    std::unique_lock<std::mutex> lk(mLockSetCurrentFunction);

    mCurrentUsbFunctions = functions;
    mCurrentUsbFunctionsApplied = true;

    ALOGI("setCurrentUsbFunctions: functions=0x%lx", (long)functions);

    if (callback) {
        ScopedAStatus ret = callback->setCurrentUsbFunctionsCb(
            functions, Status::SUCCESS, in_transactionId);
        if (!ret.isOk())
            ALOGE("Error while calling setCurrentUsbFunctionsCb %s",
                  ret.getDescription().c_str());
    }
    return ScopedAStatus::ok();
}

ScopedAStatus UsbGadget::reset(
        const shared_ptr<IUsbGadgetCallback> &callback,
        int64_t in_transactionId) {
    ALOGI("reset called");
    if (callback)
        callback->resetCb(Status::SUCCESS, in_transactionId);
    return ScopedAStatus::ok();
}

}  // namespace gadget
}  // namespace usb
}  // namespace hardware
}  // namespace android
}  // aidl
