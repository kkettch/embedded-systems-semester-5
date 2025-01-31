import cv2
import numpy as np

def check_color(frame, lower_bound, upper_bound):
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
    mask = cv2.inRange(hsv, lower_bound, upper_bound)
    return cv2.countNonZero(mask) > 0

def main():
    # TODO: Для микроконтроллера использовать другое, для Егора и Маши наверное использовать 0 вместо 1
    cap = cv2.VideoCapture(1)

    if not cap.isOpened():
        print("Не удалось открыть камеру")
        return

    # Определение цветовых границ для каждого квадрата с учетом чувствительности и насыщенности
    colors = [
        (np.array([125, 100, 100]), np.array([150, 255, 255])), # Фиолетовый
        (np.array([35, 100, 100]), np.array([85, 255, 255])),   # Зеленый
        (np.array([100, 150, 0]), np.array([140, 255, 255])),   # Синий
        (np.array([20, 100, 100]), np.array([30, 255, 255]))    # Желтый
    ]

    # Последовательность цветов для доступа
    access_sequence = [0, 1, 2, 3]
    current_index = 0
    squares_status = [False, False, False, False]
    access_granted = False

    while True:
        ret, frame = cap.read()
        if not ret:
            print("Не удалось получить кадр")
            break

        # Отразить изображение по горизонтали
        frame = cv2.flip(frame, 1)

        height, width, _ = frame.shape
        square_size = 100

        # Координаты квадратов
        squares = [
            (50, 50, 50 + square_size, 50 + square_size),
            (width - 150, 50, width - 50, 50 + square_size),
            (50, height - 150, 50 + square_size, height - 50),
            (width - 150, height - 150, width - 50, height - 50)
        ]

        for i, (x1, y1, x2, y2) in enumerate(squares):
            square = frame[y1:y2, x1:x2]
            if not squares_status[i]:
                if check_color(square, colors[i][0], colors[i][1]):
                    if i == access_sequence[current_index]:
                        squares_status[i] = True
                        current_index += 1
                        if current_index == len(access_sequence):
                            access_granted = True
                    else:
                        cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 0, 255), -1)  # Закрасить красным
                else:
                    cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 0, 255), -1)  # Закрасить красным
            else:
                cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), -1)  # Закрасить зеленым

        if access_granted:
            cv2.putText(frame, 'Access', (width // 2 - 50, height // 2), cv2.FONT_HERSHEY_SIMPLEX, 2, (0, 255, 0), 3)

        cv2.imshow('Camera', frame)

        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            break
        elif key == ord('u'):
            # Сбросить процесс
            current_index = 0
            squares_status = [False, False, False, False]
            access_granted = False

    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()