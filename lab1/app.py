import subprocess
import customtkinter as ctk

ctk.set_appearance_mode("System")
ctk.set_default_color_theme("blue")

EXE_PATH = "PSSM.exe"  


class PowerManagerTableApp(ctk.CTk):
    def __init__(self):
        super().__init__()

        self.title("Менеджер схем питания")
        self.geometry("900x500")
        self.resizable(True, True)

        self.schemes_data = []

        # Главная разметка
        self.grid_columnconfigure(0, weight=1)
        self.grid_rowconfigure(0, weight=1)

        # Таблица параметров схем питания
        self.table_frame = ctk.CTkScrollableFrame(
            self, label_text="Схемы электропитания"
        )
        self.table_frame.grid(row=0, column=0, padx=15, pady=15, sticky="nsew")

        # Нижняя панель (Статус батареи и кнопка обновить)
        self.status_bar = ctk.CTkFrame(self, height=40)
        self.status_bar.grid(row=1, column=0, padx=15, pady=(0, 15), sticky="ew")
        self.lbl_battery_status = ctk.CTkLabel(
            self.status_bar, 
            text="Статус батареи: Чтение данных...", 
            font=("Arial", 11, "bold")
        )
        self.lbl_battery_status.pack(side="left", padx=15, pady=6)

        self.btn_refresh = ctk.CTkButton(
            self.status_bar, text="Обновить", width=100, command=self.load_data
        )
        self.btn_refresh.pack(side="right", padx=10, pady=6)

        # Первоначальная загрузка данных
        self.load_data()

    def format_time(self, seconds_str):
        sec = int(seconds_str)
        if sec == 0:
            return "Никогда"
        return f"{sec // 60} мин"

    def run_cpp_exe(self, arg=None):
        cmd = [EXE_PATH]
        if arg:
            cmd.append(arg)
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, encoding="utf-8", errors="ignore")
            return result.stdout
        except Exception as e:
            return f"ERROR: {str(e)}"

    def load_data(self):
        output = self.run_cpp_exe()
        if "ERROR" in output or not output:
            self.lbl_battery_status.configure(text=f"Ошибка: Не удалось запустить {EXE_PATH}")
            return

        lines = output.splitlines()
        battery_info = []
        self.schemes_data = []

        current_scheme = {}
        in_schemes_block = False

        for line in lines:
            line = line.strip()
            if any(key in line for key in ["Тип питания:", "Уровень заряда:", "Осталось времени:"]):
                battery_info.append(line)
            elif line == "--- SCHEMES START ---":
                in_schemes_block = True
            elif in_schemes_block:
                if line.startswith("NAME:"):
                    current_scheme["name"] = line.replace("NAME:", "")
                elif line.startswith("GUID:"):
                    current_scheme["guid"] = line.replace("GUID:", "")
                elif line.startswith("ACTIVE:"):
                    current_scheme["active"] = line.replace("ACTIVE:", "") == "1"
                elif line.startswith("DISPLAY_AC:"):
                    current_scheme["disp_ac"] = line.replace("DISPLAY_AC:", "")
                elif line.startswith("DISPLAY_DC:"):
                    current_scheme["disp_dc"] = line.replace("DISPLAY_DC:", "")
                elif line.startswith("SLEEP_AC:"):
                    current_scheme["sleep_ac"] = line.replace("SLEEP_AC:", "")
                elif line.startswith("SLEEP_DC:"):
                    current_scheme["sleep_dc"] = line.replace("SLEEP_DC:", "")
                elif line.startswith("CRIT_AC:"):
                    current_scheme["crit_ac"] = line.replace("CRIT_AC:", "")
                elif line.startswith("CRIT_DC:"):
                    current_scheme["crit_dc"] = line.replace("CRIT_DC:", "")
                elif line == "--- SCHEME END ---":
                    self.schemes_data.append(current_scheme)
                    current_scheme = {}

        # Обновление строки статуса батареи
        if battery_info:
            self.lbl_battery_status.configure(text="  |  ".join(battery_info))

        # Очистка старой таблицы
        for widget in self.table_frame.winfo_children():
            widget.destroy()

        self.render_table()

    def render_table(self):
        # Четкие и понятные заголовки колонок
        headers = [
            "Режим питания", 
            "Отключение экрана (Сеть / Батарея)", 
            "Переход в сон (Сеть / Батарея)", 
            "Критический разряд (Сеть / Батарея)"
        ]

        # Конфигурация колонок по ширине
        self.table_frame.grid_columnconfigure(0, weight=2)
        for col_idx in range(1, len(headers)):
            self.table_frame.grid_columnconfigure(col_idx, weight=1)

        # Отрисовка шапки таблицы
        for col_idx, header_text in enumerate(headers):
            lbl = ctk.CTkLabel(
                self.table_frame, 
                text=header_text, 
                font=("Arial", 11, "bold"),
                fg_color=("#c0c0c0", "#2b2b2b"),
                corner_radius=10,  # Закругление шапки
                padx=10, 
                pady=8
            )
            lbl.grid(row=0, column=col_idx, padx=3, pady=4, sticky="ew")

        # Отрисовка строк со схемами
        for row_idx, scheme in enumerate(self.schemes_data, start=1):
            is_active = scheme["active"]

            # Цвета фона и текста
            cell_bg = ("#d1e7dd", "#1b4332") if is_active else ("#ffffff", "#2b2b2b")
            text_color = ("#0f5132", "#ffffff") if is_active else ("#000000", "#ffffff")

            # Значения для колонок
            name_text = f"★ {scheme['name']}" if is_active else scheme['name']
            disp_str = f"{self.format_time(scheme['disp_ac'])} / {self.format_time(scheme['disp_dc'])}"
            sleep_str = f"{self.format_time(scheme['sleep_ac'])} / {self.format_time(scheme['sleep_dc'])}"
            crit_str = f"{scheme['crit_ac']}% / {scheme['crit_dc']}%"

            row_data = [name_text, disp_str, sleep_str, crit_str]

            # Отрисовка ячеек
            for col_idx, cell_value in enumerate(row_data):
                font_weight = "bold" if is_active or col_idx == 0 else "normal"
                
                # Фрейм с явным закруглением без жесткой высоты
                cell_frame = ctk.CTkFrame(
                    self.table_frame,
                    fg_color=cell_bg,
                    corner_radius=10  # Закруглённые углы для ВСЕХ ячеек
                )
                cell_frame.grid(row=row_idx, column=col_idx, padx=3, pady=3, sticky="ew")
                cell_frame.grid_columnconfigure(0, weight=1)

                # Текст внутри с отступами pady, определяющими высоту плашки
                lbl = ctk.CTkLabel(
                    cell_frame,
                    text=cell_value,
                    font=("Arial", 12, font_weight),
                    text_color=text_color,
                    fg_color="transparent",
                    anchor="w" if col_idx == 0 else "center",
                    padx=10
                )
                lbl.grid(row=0, column=0, sticky="ew", pady=8)

                # Двойной клик для активации
                cell_frame.bind("<Double-Button-1>", lambda event, s=scheme: self.double_click_activate(s))
                lbl.bind("<Double-Button-1>", lambda event, s=scheme: self.double_click_activate(s))

    def double_click_activate(self, scheme):
        if not scheme["active"]:
            self.run_cpp_exe(scheme["guid"])
            self.load_data()


if __name__ == "__main__":
    app = PowerManagerTableApp()
    app.mainloop()