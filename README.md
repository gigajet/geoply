Yêu cầu:
+ Windows từ win7 trở lên
+ Máy tính little endian (như Intel)
+ Độ phân giải màn hình tối thiểu 1024 x 768
Nếu không thỏa mãn, có thể chạy sai.

Sử dụng: thư viện SFML, WinAPI, não và tay.

Khởi động thành công, có thể nhấn N hoặc M rồi bắt đầu.

Có 3 trạng thái:
1. Không chọn hình
2. Chọn hình, không chọn điểm
3. Chọn hình và hai điểm

==

1. Không chọn hình
- Lưu ra file nghĩa là lưu toàn bộ bàn làm việc.
Tải từ file là tải toàn bộ bàn làm việc trước đó.
Lưu ý mọi hình hiện tại sẽ bị mất.
- Dẹp hết tức là xóa hết mọi hình đang có trên bàn.
- Có thể click chuột để chọn một hình, chuyển qua trạng thái 2.

2. Chọn hình, không chọn điểm
- Có thể nhấn Delete để xóa hình.
- Có thể bấm chuột phải hoặc nhấn ESC để bỏ chọn, chuyển qua trạng thái 1.
- Có thể nhấn Tab để chọn hình khác.
- Nhấn S để lưu hình đang chọn thành file hình (hiện tại chỉ hỗ trợ BMP)
- Có thể nhấp chuột vào biên (chu vi) của hình đang chọn để chọn điểm.
Chọn điểm rồi, bấm chuột phải hoặc phím ESC để bỏ chọn.
Chọn tối đa 2 điểm sẽ qua trạng thái 3.

3. Chọn hình và hai điểm
- Có thể nhấp chuột phải hoặc ESC để bỏ chọn từng điểm.
- Nếu hai điểm chia hình làm hai phần thì có thể nhấn C để cắt hình làm hai.

