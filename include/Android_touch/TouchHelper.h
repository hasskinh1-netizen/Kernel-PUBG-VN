
#ifndef TOUCHHELPER_H
#define TOUCHHELPER_H
#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define test_bit(array, bit) ((array[bit / BITS_PER_LONG] >> bit % BITS_PER_LONG) & 1)

void HandleTouchEvent();

bool GrabTouchScreen();

enum FingerStatus {
    FINGER_NO,        // Trạng thái rảnh (không chạm)
    FINGER_X_UPDATE,  // Cập nhật tọa độ trục X
    FINGER_Y_UPDATE,  // Cập nhật tọa độ trục Y
    FINGER_XY_UPDATE, // Cập nhật đồng thời cả hai trục X và Y
    FINGER_UP         // Nhấc ngón tay lên (kết thúc chạm)
};

struct TouchFinger {
    int x = -1, y = -1;       // Tọa độ điểm chạm (X, Y)
    int tracking_id = -1;     // ID theo dõi điểm chạm đa điểm của kernel
    int status = FINGER_NO;
    timeval time;
}; // Hỗ trợ tối đa 10 điểm chạm đa điểm

void Touch_Down(int slot,int x,int y); // Thao tác nhấn chạm xuống màn hình

void Touch_Move(int slot,int x,int y); // Thao tác di chuyển ngón tay (tương đương Touch_Down)

void Touch_Up(int slot); // Thao tác nhấc ngón tay khỏi màn hình

TouchFinger* getTouchFinger(int slot);

void setTouchCallback(void (*_callback)(TouchFinger*));

void TouchScreenHandle(); // Luồng lắng nghe và xử lý sự kiện cảm ứng màn hình
	
#endif
