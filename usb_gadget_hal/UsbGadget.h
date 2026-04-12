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

#pragma once

#include <android-base/file.h>
#include <android-base/properties.h>
#include <android-base/strings.h>
#include <aidl/android/hardware/usb/gadget/BnUsbGadget.h>
#include <aidl/android/hardware/usb/gadget/BnUsbGadgetCallback.h>
#include <aidl/android/hardware/usb/gadget/GadgetFunction.h>
#include <aidl/android/hardware/usb/gadget/IUsbGadget.h>
#include <aidl/android/hardware/usb/gadget/IUsbGadgetCallback.h>
#include <utils/Log.h>
#include <mutex>
#include <string>

namespace aidl {
namespace android {
namespace hardware {
namespace usb {
namespace gadget {

using ::aidl::android::hardware::usb::gadget::GadgetFunction;
using ::aidl::android::hardware::usb::gadget::IUsbGadgetCallback;
using ::aidl::android::hardware::usb::gadget::IUsbGadget;
using ::aidl::android::hardware::usb::gadget::Status;
using ::aidl::android::hardware::usb::gadget::UsbSpeed;
using ::android::base::ReadFileToString;
using ::android::base::Trim;
using ::ndk::ScopedAStatus;
using ::std::shared_ptr;
using ::std::string;

// MTK mt6877 UDC path
#define UDC_PATH "/sys/class/udc/11201000.usb0/"
#define SPEED_PATH UDC_PATH "current_speed"

struct UsbGadget : public BnUsbGadget {
    UsbGadget();

    std::mutex mLockSetCurrentFunction;
    long mCurrentUsbFunctions;
    bool mCurrentUsbFunctionsApplied;
    UsbSpeed mUsbSpeed;

    ScopedAStatus setCurrentUsbFunctions(int64_t functions,
            const shared_ptr<IUsbGadgetCallback> &callback,
            int64_t timeoutMs, int64_t in_transactionId) override;

    ScopedAStatus getCurrentUsbFunctions(const shared_ptr<IUsbGadgetCallback> &callback,
            int64_t in_transactionId) override;

    ScopedAStatus reset(const shared_ptr<IUsbGadgetCallback> &callback,
            int64_t in_transactionId) override;

    ScopedAStatus getUsbSpeed(const shared_ptr<IUsbGadgetCallback> &callback,
            int64_t in_transactionId) override;
};

}  // namespace gadget
}  // namespace usb
}  // namespace hardware
}  // namespace android
}  // aidl
