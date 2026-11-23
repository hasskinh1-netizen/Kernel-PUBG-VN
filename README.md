# Kernel-PUBG-VN (Android Vulkan Native Overlay)

[![Language](https://img.shields.io/badge/Language-C%2B%2B17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Platform](https://img.shields.io/badge/Platform-Android%20arm64--v8a-green.svg)](https://developer.android.com/ndk)
[![Render API](https://img.shields.io/badge/Render-Vulkan-red.svg)](https://www.vulkan.org/)
[![GUI](https://img.shields.io/badge/GUI-Dear%20ImGui-orange.svg)](https://github.com/ocornut/imgui)
[![License](https://img.shields.io/badge/License-Educational%20Only-lightgrey.svg)](#disclaimer)

Dự án nghiên cứu kỹ thuật hiển thị đồ họa trực tiếp qua **Vulkan Native Surface Overlay** và xử lý mô phỏng cảm ứng Linux Input Subsystem (`/dev/input`) trên nền tảng Android 64-bit (`arm64-v8a`).

---

## 📑 Mục lục
- [1. Kiến trúc hệ thống](#1-kiến-trúc-hệ-thống)
- [2. Cấu trúc thư mục](#2-cấu-trúc-thư-mục)
- [3. Yêu cầu môi trường](#3-yêu-cầu-môi-trường)
- [4. Hướng dẫn Build với NDK](#4-hướng-dẫn-build-với-ndk)
- [5. Hướng dẫn cài đặt & Thực thi](#5-hướng-dẫn-cài-đặt--thực-thi)
- [6. Tài liệu tham khảo trong dự án](#6-tài-liệu-tham-khảo-trong-dự-án)
- [7. Disclaimer](#7-disclaimer)

---

## 1. Kiến trúc hệ thống

```
+-------------------------------------------------------------+
|                      User-space Overlay                     |
|  +-------------------------------------------------------+  |
|  |           Dear ImGui GUI + Render Pipeline            |  |
|  +-------------------------------------------------------+  |
|  |             Vulkan Native Surface Creator             |  |
|  +-------------------------------------------------------+  |
|  |     Touch Helper Simulation (/dev/input/event*)       |  |
|  +-------------------------------------------------------+  |
|  |             c_driver (IPC Kernel Client)              |  |
+------------------------------+------------------------------+
                               | Netlink 17 / Syscall Hook
+------------------------------v------------------------------+
|                        Kernel-space                         |
|   Linux Kernel Module (.ko) / KernelSU / APatch Driver      |
+-------------------------------------------------------------+
```

1. **Hiển thị (Graphics)**:
   - Sử dụng `ANativeWindowCreator` tạo surface trực tiếp từ `SurfaceFlinger` thông qua Vulkan API, không cần cấp quyền `SYSTEM_ALERT_WINDOW` truyền thống.
   - Hỗ trợ chống quay màn hình cơ bản qua cờ hệ thống.
2. **Giao diện (UI)**:
   - Tích hợp **Dear ImGui (v1.8x)** hỗ trợ menu điều khiển, vẽ radar, thông số thế giới và tọa độ ma trận.
3. **Mô phỏng cảm ứng (Input)**:
   - Module `TouchHelper` quét và đọc/ghi trực tiếp vào thiết bị sự kiện `/dev/input/event*` của màn hình cảm ứng để mô phỏng tương tác ngón tay mà không cần Accessibility Service.
4. **Giao tiếp Driver (Memory Access)**:
   - Hỗ trợ 2 chế độ IPC giữa user-space binary và kernel driver:
     - **Netlink Socket**: Sử dụng giao thức mạng nội bộ `socket(PF_NETLINK, SOCK_RAW, 17)`.
     - **Syscall Hook**: Sử dụng system call ít dùng trong kernel (`lookup_dcookie`) để chuyển đổi con trỏ đọc bộ nhớ.

---

## 2. Cấu trúc thư mục

```
jni/
├── Android.mk                  # Cấu hình biên dịch source files & NDK flags
├── Application.mk              # Khai báo ABI (arm64-v8a), C++17, STL (c++_static)
├── BAO_CAO_UPDATE_OFFSET.md    # Tài liệu kỹ thuật chi tiết về cấu trúc UE4 & Offsets
├── HUONG_DAN_CAI_DAT.md        # Hướng dẫn chi tiết biên dịch, nạp driver và gỡ lỗi
├── README.md                   # Tài liệu tổng quan dự án
├── include/                    # Header files của các thư viện phụ thuộc
│   ├── Android_draw/           # Khai báo lớp vẽ đồ họa
│   ├── Android_touch/          # Khai báo TouchHelper
│   ├── Android_vulkan/         # Wrapper Vulkan Graphics
│   ├── ImGui/                  # Thư viện Dear ImGui & Font loader
│   ├── native_surface/         # ANativeWindow interface
│   └── Tools/                  # Header hỗ trợ (c_driver, structs, matrix math)
└── src/                        # Mã nguồn C++ chính
    ├── main.cpp                # Điểm khởi chạy chương trình (Entry point)
    ├── UI.h                    # Giao diện menu ImGui
    ├── Aim/                    # Module tính toán góc ngắm & bù trừ tọa độ
    ├── Android_draw/           # Vòng lặp đọc dữ liệu UE4, tính toán ma trận
    ├── Android_touch/          # Triển khai mô phỏng cảm ứng Linux input
    ├── Android_vulkan/         # Khởi tạo Vulkan Instance & Swapchain
    └── ImGui/                  # ImGui backend bindings (Vulkan + Android)
```

---

## 3. Yêu cầu môi trường

* **Công cụ biên dịch**:
  - Android NDK (Khuyến nghị bản **r21e** đến **r25c**).
  - Hệ điều hành: Windows (PowerShell/CMD), macOS hoặc Linux.
* **Thiết bị chạy thực tế**:
  - Điện thoại Android đã root (KernelSU, APatch hoặc Magisk).
  - Kiến trúc CPU: **ARM64 (aarch64 / arm64-v8a)**.
  - Đã nạp driver kernel module (`.ko`) tương ứng với phiên bản Linux kernel của thiết bị.

---

## 4. Hướng dẫn Build với NDK

### Bước 1: Mở Terminal tại thư mục chứa dự án
Thư mục gốc phải là thư mục cha chứa thư mục `jni/` (hoặc đứng ngay trong `jni/`):

```bash
cd /path/to/project
```

### Bước 2: Chạy lệnh `ndk-build`

```bash
# Biên dịch 64-bit arm64-v8a với toàn bộ luồng CPU
ndk-build -j8
```

### Bước 3: Kiểm tra tệp thực thi tạo ra
Sau khi hoàn tất, tệp nhị phân ELF sẽ nằm tại:
```
libs/arm64-v8a/和平.sh
```
*(Ghi chú: File có đuôi `.sh` theo cấu hình tên đầu ra trong `Android.mk`, nhưng định dạng thực tế là ELF 64-bit binary).*

---

## 5. Hướng dẫn cài đặt & Thực thi

### 1. Đẩy file nhị phân vào thiết bị qua ADB
```bash
adb push libs/arm64-v8a/和平.sh /data/local/tmp/
```

### 2. Cấp quyền thực thi và khởi chạy với quyền Root
```bash
adb shell
su
cd /data/local/tmp
chmod 777 和平.sh
./和平.sh
```

### 3. Chọn chế độ driver khi được hỏi
Khi khởi động, console sẽ hiển thị thông báo chọn driver:
* Nhập `0`: Chế độ Hook Syscall (`ditpro 3.3`).
* Nhập `1`: Chế độ Netlink Raw Socket (`dit 3.3`).

---

## 6. Tài liệu tham khảo trong dự án

* [BAO_CAO_UPDATE_OFFSET.md](BAO_CAO_UPDATE_OFFSET.md): Báo cáo chi tiết từng thành phần UE4 (GNames, UWorld, PersistentLevel, AActor, SkeletalMeshComponent, Bone Matrix) cần cập nhật khi game ra bản vá mới.
* [HUONG_DAN_CAI_DAT.md](HUONG_DAN_CAI_DAT.md): Sổ tay hướng dẫn khắc phục các lỗi biên dịch NDK, lỗi thiếu driver, lỗi SurfaceFlinger permission và lỗi input device.

---

## 7. Disclaimer

> [!WARNING]
> Dự án này được tạo ra **duy nhất cho mục đích nghiên cứu học thuật**, tìm hiểu cơ chế hoạt động của Android Graphics Subsystem (Vulkan), Linux Input Subsystem, và nghiên cứu kiến trúc bảo mật bộ nhớ trong các game engine hiện đại (Unreal Engine). Nghiêm cấm mọi hành vi sử dụng nhằm mục đích gian lận thương mại hoặc gây ảnh hưởng tiêu cực tới môi trường trò chơi trực tuyến.
