import numpy as np
import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'Plasma/module1'))

from plasma_model_2D import*

import matplotlib

matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import tkinter as tk
from tkinter import ttk, messagebox, filedialog
import json
from typing import Dict


class InductorPlasmaCalculatorGUI:
    """GUI для расчета интегральных параметров ВЧИ плазмотрона"""

    def __init__(self, root):
        self.root = root
        self.root.title("Расчет параметров ВЧИ плазмотрона")
        self.root.geometry("1400x900")

        # Переменные для хранения данных
        self.temperature_field = None
        self.current_results = None

        # Значения по умолчанию
        self.default_params = {
            'frequency_MHz': 27.12,
            'plasmotron_length_mm': 47.0,
            'inductor_inner_diameter_mm': 34.0,
            'inductor_height_mm': 24.0,
            'inductor_wire_diameter_mm': 5.0,
            'inductor_turns': 3,
            'coil_distance_from_inlet_mm': 7.0,
            'inductor_current_A': 150.0,
            'gas_type': 'Аргон',

            # Параметры трубок (3 возможных)
            'tube1_active': True,
            'tube1_inner_diameter_mm': 3.0,
            'tube1_outer_diameter_mm': 5.5,
            'tube1_flow_rate_lpm': 0.0,

            'tube2_active': True,
            'tube2_inner_diameter_mm': 18.0,
            'tube2_outer_diameter_mm': 21.6,
            'tube2_flow_rate_lpm': 2.5,

            'tube3_active': False,  # по умолчанию неактивна
            'tube3_inner_diameter_mm': 24.0,
            'tube3_outer_diameter_mm': 29.0,
            'tube3_flow_rate_lpm': 20.0,
        }

        # Создаем интерфейс
        self.create_widgets()

    def create_widgets(self):
        """Создание всех элементов интерфейса"""
        # Создаем Notebook для вкладок
        self.notebook = ttk.Notebook(self.root)
        self.notebook.pack(fill='both', expand=True, padx=10, pady=10)

        # Вкладки
        self.create_input_tab()
        self.create_temperature_tab()
        self.create_results_tab()
        self.create_visualization_tab()

    def create_input_tab(self):
        """Вкладка для ввода параметров"""
        input_frame = ttk.Frame(self.notebook)
        self.notebook.add(input_frame, text="Параметры")

        # Canvas для скроллинга
        canvas = tk.Canvas(input_frame)
        scrollbar = ttk.Scrollbar(input_frame, orient="vertical", command=canvas.yview)
        scrollable_frame = ttk.Frame(canvas)

        scrollable_frame.bind(
            "<Configure>",
            lambda e: canvas.configure(scrollregion=canvas.bbox("all"))
        )

        canvas.create_window((0, 0), window=scrollable_frame, anchor="nw")
        canvas.configure(yscrollcommand=scrollbar.set)

        # Основные параметры
        main_group = ttk.LabelFrame(scrollable_frame, text="Основные параметры", padding=10)
        main_group.pack(fill="x", padx=5, pady=5)

        self.create_param_entry(main_group, "Частота, МГц:", "frequency_MHz", 0)
        self.create_param_entry(main_group, "Ток индуктора, А:", "inductor_current_A", 1)
        self.create_param_entry(main_group, "Длина плазмотрона, мм:", "plasmotron_length_mm", 2)
        self.create_param_entry(main_group, "Внутренний диаметр индуктора, мм:", "inductor_inner_diameter_mm", 3)

        # Параметры индуктора
        inductor_group = ttk.LabelFrame(scrollable_frame, text="Параметры индуктора", padding=10)
        inductor_group.pack(fill="x", padx=5, pady=5)

        self.create_param_entry(inductor_group, "Высота индуктора, мм:", "inductor_height_mm", 0)
        self.create_param_entry(inductor_group, "Диаметр витков, мм:", "inductor_wire_diameter_mm", 1)
        self.create_param_entry(inductor_group, "Число витков:", "inductor_turns", 2)
        self.create_param_entry(inductor_group, "Расстояние от входа, мм:", "coil_distance_from_inlet_mm", 3)

        # Газ
        gas_group = ttk.LabelFrame(scrollable_frame, text="Плазмообразующий газ", padding=10)
        gas_group.pack(fill="x", padx=5, pady=5)

        ttk.Label(gas_group, text="Тип газа:").grid(row=0, column=0, sticky="w", padx=5, pady=2)
        self.gas_var = tk.StringVar(value=self.default_params['gas_type'])
        gas_combo = ttk.Combobox(gas_group, textvariable=self.gas_var,
                                 values=["Аргон", "Азот", "Гелий", "Водород", "Кислород"],
                                 state="readonly", width=15)
        gas_combo.grid(row=0, column=1, sticky="w", padx=5, pady=2)

        # Параметры трубок
        self.tube_frames = []
        for i in range(1, 4):
            tube_group = ttk.LabelFrame(scrollable_frame, text=f"Трубка {i}", padding=10)
            tube_group.pack(fill="x", padx=5, pady=5)

            # Галочка активности
            active_var = tk.BooleanVar(value=self.default_params[f'tube{i}_active'])
            setattr(self, f'tube{i}_active_var', active_var)

            checkbox = ttk.Checkbutton(tube_group, text="Использовать", variable=active_var,
                                       command=lambda idx=i: self.toggle_tube_fields(idx))
            checkbox.grid(row=0, column=0, columnspan=2, sticky="w", padx=5, pady=2)

            # Поля параметров
            self.create_param_entry(tube_group, "Внутренний диаметр, мм:", f"tube{i}_inner_diameter_mm", 1, tube_idx=i)
            self.create_param_entry(tube_group, "Внешний диаметр, мм:", f"tube{i}_outer_diameter_mm", 2, tube_idx=i)
            self.create_param_entry(tube_group, "Расход газа, л/мин:", f"tube{i}_flow_rate_lpm", 3, tube_idx=i)

            self.tube_frames.append(tube_group)

        # Кнопки управления
        button_frame = ttk.Frame(scrollable_frame)
        button_frame.pack(fill="x", padx=5, pady=20)

        ttk.Button(button_frame, text="Загрузить по умолчанию",
                   command=self.load_defaults).pack(side="left", padx=5)
        ttk.Button(button_frame, text="Сохранить параметры",
                   command=self.save_params).pack(side="left", padx=5)
        ttk.Button(button_frame, text="Загрузить параметры",
                   command=self.load_params).pack(side="left", padx=5)
        ttk.Button(button_frame, text="Выполнить расчет",
                   command=self.calculate_parameters, style="Accent.TButton").pack(side="right", padx=5)

        # Настройка стиля для кнопки расчета
        style = ttk.Style()
        style.configure("Accent.TButton", font=("Arial", 10, "bold"))

        canvas.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")

        # Инициализация состояния полей трубок
        for i in range(1, 4):
            self.toggle_tube_fields(i)

    def create_param_entry(self, parent, label_text, param_name, row, tube_idx=None):
        """Создание поля ввода для параметра"""
        ttk.Label(parent, text=label_text).grid(row=row, column=0, sticky="w", padx=5, pady=2)

        # Создаем переменную и поле ввода
        var = tk.DoubleVar(value=self.default_params[param_name])
        setattr(self, f"{param_name}_var", var)

        entry = ttk.Entry(parent, textvariable=var, width=12)
        entry.grid(row=row, column=1, sticky="w", padx=5, pady=2)

        # Сохраняем ссылку на поле для управления активностью
        if tube_idx:
            setattr(self, f"{param_name}_entry", entry)

        return entry

    def toggle_tube_fields(self, tube_idx):
        """Включение/выключение полей трубки"""
        active = getattr(self, f'tube{tube_idx}_active_var').get()

        # Получаем все поля этой трубки
        params = [f'tube{tube_idx}_inner_diameter_mm',
                  f'tube{tube_idx}_outer_diameter_mm',
                  f'tube{tube_idx}_flow_rate_lpm']

        for param in params:
            entry = getattr(self, f"{param}_entry", None)
            if entry:
                entry.configure(state='normal' if active else 'disabled')

    def create_temperature_tab(self):
        """Вкладка для загрузки температурного поля"""
        temp_frame = ttk.Frame(self.notebook)
        self.notebook.add(temp_frame, text="Температурное поле")

        # Панель загрузки
        load_frame = ttk.LabelFrame(temp_frame, text="Загрузка температурного поля", padding=15)
        load_frame.pack(fill="x", padx=20, pady=10)

        ttk.Button(load_frame, text="Загрузить из файла (.npy)",
                   command=self.load_temperature_npy, width=25).pack(pady=5)
        ttk.Button(load_frame, text="Загрузить из файла (.txt)",
                   command=self.load_temperature_txt, width=25).pack(pady=5)
        ttk.Button(load_frame, text="Загрузить из 2D расчета",
                   command=self.load_from_2d_calc, width=25).pack(pady=5)

        # Информация о загруженном поле
        info_frame = ttk.LabelFrame(temp_frame, text="Информация о температурном поле", padding=15)
        info_frame.pack(fill="x", padx=20, pady=10)

        self.temp_info_text = tk.Text(info_frame, height=8, width=60, state="disabled")
        self.temp_info_text.pack()

        # Визуализация температурного поля
        viz_frame = ttk.LabelFrame(temp_frame, text="Визуализация", padding=15)
        viz_frame.pack(fill="both", expand=True, padx=20, pady=10)

        self.temp_fig, self.temp_ax = plt.subplots(figsize=(8, 4))
        self.temp_canvas = FigureCanvasTkAgg(self.temp_fig, viz_frame)
        self.temp_canvas.get_tk_widget().pack(fill="both", expand=True)

        # Начальное сообщение
        self.update_temp_info("Температурное поле не загружено")

    def create_results_tab(self):
        """Вкладка для отображения результатов"""
        results_frame = ttk.Frame(self.notebook)
        self.notebook.add(results_frame, text="Результаты")

        # Текстовое поле для результатов
        text_frame = ttk.Frame(results_frame)
        text_frame.pack(fill="both", expand=True, padx=10, pady=10)

        # Создаем Text виджет с прокруткой
        self.results_text = tk.Text(text_frame, wrap=tk.WORD, font=("Courier", 10))
        scrollbar = ttk.Scrollbar(text_frame, command=self.results_text.yview)
        self.results_text.configure(yscrollcommand=scrollbar.set)

        self.results_text.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")

        # Кнопки для работы с результатами
        button_frame = ttk.Frame(results_frame)
        button_frame.pack(fill="x", padx=10, pady=5)

        ttk.Button(button_frame, text="Очистить",
                   command=self.clear_results).pack(side="left", padx=5)
        ttk.Button(button_frame, text="Сохранить в файл",
                   command=self.save_results).pack(side="left", padx=5)
        ttk.Button(button_frame, text="Копировать",
                   command=self.copy_results).pack(side="left", padx=5)

    def create_visualization_tab(self):
        """Вкладка для визуализации расчетов"""
        viz_frame = ttk.Frame(self.notebook)
        self.notebook.add(viz_frame, text="Графики")

        # Основной график
        self.viz_fig, self.viz_axes = plt.subplots(2, 2, figsize=(10, 8))
        self.viz_canvas = FigureCanvasTkAgg(self.viz_fig, viz_frame)
        self.viz_canvas.get_tk_widget().pack(fill="both", expand=True, padx=10, pady=10)

        # Кнопки для графиков
        button_frame = ttk.Frame(viz_frame)
        button_frame.pack(fill="x", padx=10, pady=5)

        ttk.Button(button_frame, text="Обновить графики",
                   command=self.update_plots).pack(side="left", padx=5)
        ttk.Button(button_frame, text="Сохранить графики",
                   command=self.save_plots).pack(side="left", padx=5)

        # Начальное сообщение на графиках
        for ax in self.viz_axes.flatten():
            ax.text(0.5, 0.5, "Загрузите данные\nи выполните расчет",
                    ha='center', va='center', transform=ax.transAxes, fontsize=12)
            ax.set_xticks([])
            ax.set_yticks([])

        self.viz_canvas.draw()

    def get_input_params(self):
        """Получение параметров из полей ввода"""
        params = {}

        # Основные параметры
        basic_params = ['frequency_MHz', 'inductor_current_A', 'plasmotron_length_mm',
                        'inductor_inner_diameter_mm', 'inductor_height_mm',
                        'inductor_wire_diameter_mm', 'inductor_turns',
                        'coil_distance_from_inlet_mm']

        for param in basic_params:
            var = getattr(self, f"{param}_var")
            params[param] = var.get()

        # Газ
        params['gas_type'] = self.gas_var.get()

        # Параметры трубок
        for i in range(1, 4):
            active = getattr(self, f'tube{i}_active_var').get()
            params[f'tube{i}_active'] = active

            if active:
                tube_params = [f'tube{i}_inner_diameter_mm', f'tube{i}_outer_diameter_mm',
                               f'tube{i}_flow_rate_lpm']
                for param in tube_params:
                    var = getattr(self, f"{param}_var")
                    params[param] = var.get()

        return params

    def load_defaults(self):
        """Загрузка параметров по умолчанию"""
        for param_name, value in self.default_params.items():
            if param_name == 'gas_type':
                self.gas_var.set(value)
            elif param_name.endswith('_active'):
                getattr(self, f"{param_name}_var").set(value)
            elif param_name in self.default_params:
                var = getattr(self, f"{param_name}_var", None)
                if var:
                    var.set(value)

        # Обновляем состояние полей
        for i in range(1, 4):
            self.toggle_tube_fields(i)

        messagebox.showinfo("Успех", "Параметры загружены по умолчанию")

    def save_params(self):
        """Сохранение параметров в файл"""
        params = self.get_input_params()

        filename = filedialog.asksaveasfilename(
            defaultextension=".json",
            filetypes=[("JSON files", "*.json"), ("All files", "*.*")],
            title="Сохранить параметры"
        )

        if filename:
            try:
                with open(filename, 'w', encoding='utf-8') as f:
                    json.dump(params, f, indent=2, ensure_ascii=False)
                messagebox.showinfo("Успех", f"Параметры сохранены в {filename}")
            except Exception as e:
                messagebox.showerror("Ошибка", f"Не удалось сохранить: {str(e)}")

    def load_params(self):
        """Загрузка параметров из файла"""
        filename = filedialog.askopenfilename(
            filetypes=[("JSON files", "*.json"), ("All files", "*.*")],
            title="Загрузить параметры"
        )

        if filename:
            try:
                with open(filename, 'r', encoding='utf-8') as f:
                    params = json.load(f)

                # Загружаем параметры в интерфейс
                for param_name, value in params.items():
                    if param_name == 'gas_type':
                        self.gas_var.set(value)
                    elif param_name.endswith('_active'):
                        getattr(self, f"{param_name}_var").set(value)
                    elif param_name in self.default_params:
                        var = getattr(self, f"{param_name}_var", None)
                        if var:
                            var.set(value)

                # Обновляем состояние полей
                for i in range(1, 4):
                    self.toggle_tube_fields(i)

                messagebox.showinfo("Успех", f"Параметры загружены из {filename}")
            except Exception as e:
                messagebox.showerror("Ошибка", f"Не удалось загрузить: {str(e)}")

    def load_temperature_npy(self):
        """Загрузка температурного поля из .npy файла"""
        filename = filedialog.askopenfilename(
            filetypes=[("NumPy files", "*.npy"), ("All files", "*.*")],
            title="Загрузить температурное поле"
        )

        if filename:
            try:
                self.temperature_field = np.load(filename)
                self.update_temp_display()
                messagebox.showinfo("Успех", f"Температурное поле загружено из {filename}")
            except Exception as e:
                messagebox.showerror("Ошибка", f"Не удалось загрузить: {str(e)}")

    def load_temperature_txt(self):
        """Загрузка температурного поля из текстового файла"""
        filename = filedialog.askopenfilename(
            filetypes=[("Text files", "*.txt"), ("All files", "*.*")],
            title="Загрузить температурное поле"
        )

        if filename:
            try:
                self.temperature_field = np.loadtxt(filename)
                self.update_temp_display()
                messagebox.showinfo("Успех", f"Температурное поле загружено из {filename}")
            except Exception as e:
                messagebox.showerror("Ошибка", f"Не удалось загрузить: {str(e)}")

    def load_from_2d_calc(self):
        """Загрузка температурного поля из 2D расчета"""
        try:
            # Импортируем ваш 2D расчет
            import sys
            from pathlib import Path

            # Добавляем путь к вашему модулю
            current_dir = Path(__file__).parent
            sys.path.insert(0, str(current_dir))

            from plasma_model_2D import run_simulation, Config

            # Запрашиваем параметры для 2D расчета
            dialog = tk.Toplevel(self.root)
            dialog.title("Параметры 2D расчета")
            dialog.geometry("300x200")
            dialog.transient(self.root)
            dialog.grab_set()

            tk.Label(dialog, text="Ток дуги (A):").pack(pady=5)
            i_arc_var = tk.DoubleVar(value=150.0)
            tk.Entry(dialog, textvariable=i_arc_var).pack(pady=5)

            tk.Label(dialog, text="Скорость газа (м/с):").pack(pady=5)
            v_avg_var = tk.DoubleVar(value=800.0)
            tk.Entry(dialog, textvariable=v_avg_var).pack(pady=5)

            def run_and_close():
                conf = Config(dimensions=2)
                conf.I_arc = i_arc_var.get()
                conf.V_avg = v_avg_var.get()

                # Запускаем расчет
                self.temperature_field = run_simulation(conf)
                self.update_temp_display()
                dialog.destroy()
                messagebox.showinfo("Успех", "2D расчет завершен")

            tk.Button(dialog, text="Рассчитать", command=run_and_close).pack(pady=10)

        except Exception as e:
            messagebox.showerror("Ошибка", f"Не удалось выполнить 2D расчет: {str(e)}")

    def update_temp_display(self):
        """Обновление информации о температурном поле с использованием plot_and_save_results"""
        if self.temperature_field is not None:
            try:
                # Импортируем вашу функцию
                from plasma_model_2D import plot_and_save_results

                # Создаем фиктивный объект Config для передачи в функцию
                class TempConfig:
                    def __init__(self, temperature_field):
                        self.Nz = temperature_field.shape[0]
                        self.Nx = temperature_field.shape[1]
                        self.L_z = 0.1  # примерные значения, можно настроить
                        self.L_x = 0.024
                        self.I_arc = 150.0  # из параметров 2D расчета
                        self.V_avg = 800.0  # из параметров 2D расчета

                # Создаем конфиг
                temp_conf = TempConfig(self.temperature_field)

                # Обновляем статистику
                T = self.temperature_field
                info = f"╔══════════════════════════════════════════╗\n"
                info += f"║         ТЕМПЕРАТУРНОЕ ПОЛЕ              ║\n"
                info += f"╠══════════════════════════════════════════╣\n"
                info += f"║ Размерность: {str(T.shape):<28} ║\n"
                info += f"║ Минимальная: {np.min(T):8.1f} K{'':18} ║\n"
                info += f"║ Максимальная: {np.max(T):8.1f} K{'':18} ║\n"
                info += f"║ Средняя:     {np.mean(T):8.1f} K{'':18} ║\n"
                info += f"║ Медианная:   {np.median(T):8.1f} K{'':18} ║\n"
                info += f"║ Ток дуги:    {temp_conf.I_arc:8.1f} A{'':18} ║\n"
                info += f"║ Скорость газа: {temp_conf.V_avg:6.1f} м/с{'':16} ║\n"
                info += f"╚══════════════════════════════════════════╝"

                # Обновляем информацию
                self.update_temp_info(info)

                # Обновляем график с помощью вашей функции
                self.temp_ax.clear()

                # Используем вашу функцию для построения на нашем axes
                plot_and_save_results(
                    self.temperature_field,
                    temp_conf,
                    output_path=None,  # Не сохраняем в файл
                    ax=self.temp_ax  # Передаем наш axes для рисования
                )

                # Настраиваем размер графика для GUI
                self.temp_ax.set_position([0.1, 0.1, 0.7, 0.8])  # [left, bottom, width, height]

                # Убираем лишние элементы для компактности
                self.temp_ax.set_title(f"Поперечный разрез плазмотрона\nI={temp_conf.I_arc}A, V={temp_conf.V_avg} м/с",
                                       fontsize=10)

                self.temp_canvas.draw()

            except ImportError:
                # Если не удалось импортировать, используем простой график
                self.fallback_temp_display()
            except Exception as e:
                print(f"Ошибка при построении графика: {e}")
                self.fallback_temp_display()

    def fallback_temp_display(self):
        """Простой график температуры если не удалось использовать plot_and_save_results"""
        if self.temperature_field is None:
            return

        T = self.temperature_field

        # Статистика
        info = f"Размерность: {T.shape}\n"
        info += f"Минимальная температура: {np.min(T):.1f} K\n"
        info += f"Максимальная температура: {np.max(T):.1f} K\n"
        info += f"Средняя температура: {np.mean(T):.1f} K\n"
        info += f"Медианная температура: {np.median(T):.1f} K"

        self.update_temp_info(info)

        # Простой график
        self.temp_ax.clear()

        # Используем только часть данных если массив большой
        if T.shape[0] > 100 or T.shape[1] > 100:
            step_r = max(1, T.shape[0] // 100)
            step_z = max(1, T.shape[1] // 100)
            T_display = T[::step_r, ::step_z]
        else:
            T_display = T

        im = self.temp_ax.imshow(T_display, cmap='hot', aspect='auto',
                                 interpolation='bilinear')
        self.temp_ax.set_title("Температурное поле (2D расчет)")
        self.temp_ax.set_xlabel("Ось Z (индексы)")
        self.temp_ax.set_ylabel("Ось R (индексы)")

        cbar = self.temp_fig.colorbar(im, ax=self.temp_ax)
        cbar.set_label('Температура, K')

        # Добавляем сетку
        self.temp_ax.grid(True, alpha=0.3, linestyle='--', linewidth=0.5)

        self.temp_canvas.draw()

    def update_temp_info(self, text):
        """Обновление текстовой информации"""
        self.temp_info_text.config(state="normal")
        self.temp_info_text.delete(1.0, tk.END)
        self.temp_info_text.insert(1.0, text)
        self.temp_info_text.config(state="disabled")

    def calculate_parameters(self):
        """Выполнение расчета интегральных параметров"""
        if self.temperature_field is None:
            messagebox.showwarning("Внимание", "Сначала загрузите температурное поле!")
            self.notebook.select(1)  # Переходим на вкладку температурного поля
            return

        try:
            # Получаем параметры
            input_params = self.get_input_params()

            # Создаем калькулятор и выполняем расчет
            calculator = InductorPlasmaCalculator(input_params, self.temperature_field)
            self.current_results = calculator.calculate_integral_parameters()

            # Обновляем результаты
            self.update_results_display()

            # Обновляем графики
            self.update_plots()

            messagebox.showinfo("Успех", "Расчет завершен успешно!")

        except Exception as e:
            messagebox.showerror("Ошибка", f"Ошибка расчета: {str(e)}")

    def update_results_display(self):
        """Обновление отображения результатов"""
        if self.current_results is None:
            return

        # Форматируем текст результатов
        text = "=" * 80 + "\n"
        text += "ИНТЕГРАЛЬНЫЕ ЭЛЕКТРОМАГНИТНЫЕ ПАРАМЕТРЫ ВЧИ ПЛАЗМОТРОНА\n"
        text += "=" * 80 + "\n\n"

        params_list = [
            ("Частота тока индуктора", self.current_results['frequency_MHz'], "МГц"),
            ("Ток индуктора", self.current_results['inductor_current_A'], "А"),
            ("Фаза тока индуктора", self.current_results['inductor_current_phase_deg'], "°"),
            ("Выделяемая в индукторе активная мощность", self.current_results['inductor_active_power_kW'], "кВт"),
            ("Напряжение на индукторе", self.current_results['inductor_voltage_kV'], "кВ"),
            ("Фаза напряжения на индукторе", self.current_results['inductor_voltage_phase_deg'], "°"),
            ("Ток в плазме", self.current_results['plasma_current_A'], "А"),
            ("Фаза тока в плазме", self.current_results['plasma_current_phase_deg'], "°"),
            ("Выделяемая в плазме активная мощность", self.current_results['plasma_active_power_kW'], "кВт"),
            ("Коэффициент трансформации", self.current_results['transformation_ratio'], ""),
            ("Эффективное активное сопротивление", self.current_results['effective_resistance_ohm'], "Ом"),
            ("Эффективное реактивное сопротивление", self.current_results['effective_reactance_ohm'], "Ом"),
            ("Добротность", self.current_results['quality_factor'], "")
        ]

        for i, (name, value, unit) in enumerate(params_list, 1):
            text += f"{i:2d}. {name:45} {value:10.3f} {unit}\n"

        text += "\n" + "=" * 80

        # Обновляем текстовое поле
        self.results_text.delete(1.0, tk.END)
        self.results_text.insert(1.0, text)

    def update_plots(self):
        """Обновление графиков"""
        if self.temperature_field is None or self.current_results is None:
            return

        # Очищаем графики
        for ax in self.viz_axes.flatten():
            ax.clear()

        # График 1: Распределение температуры
        ax1 = self.viz_axes[0, 0]
        im1 = ax1.imshow(self.temperature_field, cmap='hot', aspect='auto')
        ax1.set_title('Температурное поле')
        ax1.set_xlabel('Ось Z')
        ax1.set_ylabel('Ось R')
        self.viz_fig.colorbar(im1, ax=ax1, label='Температура (K)')

        # График 2: Мощность vs ток
        ax2 = self.viz_axes[0, 1]
        power_data = [self.current_results['inductor_active_power_kW'],
                      self.current_results['plasma_active_power_kW']]
        labels = ['Индуктор', 'Плазма']
        colors = ['blue', 'red']
        ax2.bar(labels, power_data, color=colors)
        ax2.set_title('Активная мощность')
        ax2.set_ylabel('Мощность, кВт')
        ax2.grid(True, alpha=0.3)

        # График 3: Токи
        ax3 = self.viz_axes[1, 0]
        current_data = [self.current_results['inductor_current_A'],
                        self.current_results['plasma_current_A']]
        ax3.bar(labels, current_data, color=colors)
        ax3.set_title('Токи')
        ax3.set_ylabel('Ток, А')
        ax3.grid(True, alpha=0.3)

        # График 4: Импеданс и добротность
        ax4 = self.viz_axes[1, 1]
        impedance_data = [self.current_results['effective_resistance_ohm'],
                          self.current_results['effective_reactance_ohm'],
                          self.current_results['quality_factor']]
        impedance_labels = ['R, Ом', 'X, Ом', 'Q']
        impedance_colors = ['green', 'orange', 'purple']
        ax4.bar(impedance_labels, impedance_data, color=impedance_colors)
        ax4.set_title('Импеданс и добротность')
        ax4.grid(True, alpha=0.3)

        # Настраиваем макет
        self.viz_fig.tight_layout()
        self.viz_canvas.draw()

    def clear_results(self):
        """Очистка результатов"""
        self.results_text.delete(1.0, tk.END)
        self.current_results = None

    def save_results(self):
        """Сохранение результатов в файл"""
        if self.current_results is None:
            messagebox.showwarning("Внимание", "Нет результатов для сохранения!")
            return

        filename = filedialog.asksaveasfilename(
            defaultextension=".json",
            filetypes=[("JSON files", "*.json"), ("Text files", "*.txt"), ("All files", "*.*")],
            title="Сохранить результаты"
        )

        if filename:
            try:
                if filename.endswith('.json'):
                    # Сохраняем как JSON
                    data_to_save = {
                        'input_parameters': self.get_input_params(),
                        'temperature_field_info': {
                            'shape': self.temperature_field.shape if self.temperature_field is not None else None,
                            'min_temp': float(
                                np.min(self.temperature_field)) if self.temperature_field is not None else None,
                            'max_temp': float(
                                np.max(self.temperature_field)) if self.temperature_field is not None else None,
                            'mean_temp': float(
                                np.mean(self.temperature_field)) if self.temperature_field is not None else None
                        },
                        'integral_parameters': self.current_results
                    }

                    with open(filename, 'w', encoding='utf-8') as f:
                        json.dump(data_to_save, f, indent=2, ensure_ascii=False)

                else:
                    # Сохраняем как текст
                    with open(filename, 'w', encoding='utf-8') as f:
                        f.write(self.results_text.get(1.0, tk.END))

                messagebox.showinfo("Успех", f"Результаты сохранены в {filename}")

            except Exception as e:
                messagebox.showerror("Ошибка", f"Не удалось сохранить: {str(e)}")

    def copy_results(self):
        """Копирование результатов в буфер обмена"""
        if self.current_results is None:
            messagebox.showwarning("Внимание", "Нет результатов для копирования!")
            return

        self.root.clipboard_clear()
        self.root.clipboard_append(self.results_text.get(1.0, tk.END))
        messagebox.showinfo("Успех", "Результаты скопированы в буфер обмена")

    def save_plots(self):
        """Сохранение графиков"""
        if self.current_results is None:
            messagebox.showwarning("Внимание", "Нет данных для сохранения графиков!")
            return

        filename = filedialog.asksaveasfilename(
            defaultextension=".png",
            filetypes=[("PNG files", "*.png"), ("PDF files", "*.pdf"),
                       ("SVG files", "*.svg"), ("All files", "*.*")],
            title="Сохранить графики"
        )

        if filename:
            try:
                self.viz_fig.savefig(filename, dpi=300, bbox_inches='tight')
                messagebox.showinfo("Успех", f"Графики сохранены в {filename}")
            except Exception as e:
                messagebox.showerror("Ошибка", f"Не удалось сохранить графики: {str(e)}")


# Класс расчета (упрощенная версия для интеграции с GUI)
class InductorPlasmaCalculator:
    """Калькулятор интегральных параметров ВЧИ плазмотрона"""

    def __init__(self, input_params: Dict, temperature_field: np.ndarray):
        self.params = self._validate_input_params(input_params)
        self.T = temperature_field

        if len(self.T.shape) != 2:
            raise ValueError(f"Температурное поле должно быть 2D массивом")

        self.nr, self.nz = self.T.shape
        self.mu0 = 4e-7 * np.pi

        self.sigma = None
        self.E_field = None
        self.Q_joule = None

    def _validate_input_params(self, params: Dict) -> Dict:
        """Проверка и заполнение параметров"""
        # Основные параметры
        default_params = {
            'frequency_MHz': 27.12,
            'plasmotron_length_mm': 47.0,
            'inductor_inner_diameter_mm': 34.0,
            'inductor_height_mm': 24.0,
            'inductor_wire_diameter_mm': 5.0,
            'inductor_turns': 3,
            'coil_distance_from_inlet_mm': 7.0,
            'inductor_current_A': 150.0,
            'gas_type': 'Аргон',
        }

        # Добавляем параметры трубок если они активны
        for i in range(1, 4):
            if params.get(f'tube{i}_active', False):
                tube_defaults = {
                    f'tube{i}_inner_diameter_mm': [3.0, 18.0, 24.0][i - 1],
                    f'tube{i}_outer_diameter_mm': [5.5, 21.6, 29.0][i - 1],
                    f'tube{i}_flow_rate_lpm': [0.0, 2.5, 20.0][i - 1],
                }
                default_params.update(tube_defaults)

        # Обновляем переданными параметрами
        for key, default_value in default_params.items():
            if key not in params:
                params[key] = default_value

        return params

    def calculate_plasma_conductivity(self) -> np.ndarray:
        """Расчет электропроводности плазмы"""
        T = self.T

        sigma = np.zeros_like(T)

        mask_low = T < 6000
        sigma[mask_low] = 5 * (T[mask_low] / 6000) ** 4 * 100

        mask_mid = (T >= 6000) & (T < 10000)
        sigma[mask_mid] = 100 + 400 * ((T[mask_mid] - 6000) / 4000) ** 1.8

        mask_high = T >= 10000
        sigma[mask_high] = 500 + 500 * ((T[mask_high] - 10000) / 5000) ** 0.7

        sigma = np.clip(sigma, 1, 1200)
        self.sigma = sigma
        return sigma

    def calculate_inductor_geometry(self):
        """Расчет геометрических параметров"""
        L = float(self.params['plasmotron_length_mm']) / 1000
        R_coil = float(self.params['inductor_inner_diameter_mm']) / 2000
        h_coil = float(self.params['inductor_height_mm']) / 1000
        z_coil_start = float(self.params['coil_distance_from_inlet_mm']) / 1000

        return L, R_coil, h_coil, z_coil_start

    def calculate_inductor_field(self, r: np.ndarray, z: np.ndarray) -> np.ndarray:
        """Расчет электрического поля"""
        L, R_coil, h_coil, z_coil_start = self.calculate_inductor_geometry()
        N = int(self.params['inductor_turns'])
        I_coil = float(self.params['inductor_current_A'])
        f = float(self.params['frequency_MHz']) * 1e6
        omega = 2 * np.pi * f

        z_coil_end = z_coil_start + h_coil

        r_safe = np.where(r == 0, 1e-10, r)

        alpha1 = (z - z_coil_start) / np.sqrt(r_safe ** 2 + R_coil ** 2)
        alpha2 = (z - z_coil_end) / np.sqrt(r_safe ** 2 + R_coil ** 2)

        Bz_simplified = (self.mu0 * N * I_coil / (2 * h_coil) *
                         (alpha1 / np.sqrt(1 + alpha1 ** 2) -
                          alpha2 / np.sqrt(1 + alpha2 ** 2)))

        A_theta = 0.5 * r_safe * Bz_simplified
        E_theta = omega * np.abs(A_theta)

        if self.sigma is not None:
            skin_depth = np.sqrt(2 / (omega * self.mu0 * np.maximum(self.sigma, 1e-6)))
            attenuation = np.exp(-r_safe / np.maximum(skin_depth, 1e-6))
            E_theta *= attenuation

        return E_theta

    def calculate_joule_heating(self) -> float:
        """Расчет мощности нагрева"""
        L, R_coil, _, _ = self.calculate_inductor_geometry()

        r_coords = np.linspace(0, R_coil, self.nr)
        z_coords = np.linspace(0, L, self.nz)

        R, Z = np.meshgrid(r_coords, z_coords, indexing='ij')

        if self.sigma is None:
            self.calculate_plasma_conductivity()

        E_field = self.calculate_inductor_field(R, Z)
        self.E_field = E_field

        Q_joule = 0.5 * self.sigma * (E_field ** 2)
        self.Q_joule = Q_joule

        dr = R_coil / self.nr
        dz = L / self.nz

        r_edges = np.linspace(0, R_coil, self.nr + 1)
        volumes = np.zeros((self.nr, self.nz))

        for i in range(self.nr):
            r_inner = r_edges[i]
            r_outer = r_edges[i + 1]
            area = np.pi * (r_outer ** 2 - r_inner ** 2)
            volumes[i, :] = area * dz

        total_power = np.sum(Q_joule * volumes)
        return total_power

    def calculate_integral_parameters(self) -> Dict:
        """Расчет всех 13 интегральных параметров"""
        total_power = self.calculate_joule_heating()

        L, R_coil, _, _ = self.calculate_inductor_geometry()
        dr = R_coil / self.nr
        dz = L / self.nz

        r_coords = np.linspace(dr / 2, R_coil - dr / 2, self.nr)

        plasma_current = 0.0
        for i in range(self.nr):
            r = r_coords[i]
            J_avg = np.mean(self.sigma[i, :] * self.E_field[i, :])
            plasma_current += J_avg * 2 * np.pi * r * dr * dz

        I_coil = float(self.params['inductor_current_A'])
        f_MHz = float(self.params['frequency_MHz'])

        N = int(self.params['inductor_turns'])
        R_coil_ohm = 0.05 * N
        X_coil_ohm = 2 * np.pi * f_MHz * 1e6 * N * 0.5e-6

        Z_coil = np.sqrt(R_coil_ohm ** 2 + X_coil_ohm ** 2)
        V_coil = I_coil * Z_coil
        P_coil = I_coil ** 2 * R_coil_ohm

        # Учет активных трубок
        active_tubes = sum(1 for i in range(1, 4) if self.params.get(f'tube{i}_active', False))
        tube_factor = 1.0 / (0.8 + 0.1 * active_tubes)

        total_power *= tube_factor
        plasma_current *= tube_factor

        R_eff = total_power / (I_coil ** 2) if I_coil > 0 else 0
        Q_factor = X_coil_ohm / R_eff if R_eff > 0 else 0

        phi_current = 0.0
        phi_voltage = np.arctan2(X_coil_ohm, R_coil_ohm) * 180 / np.pi
        phi_plasma_current = 180 - phi_voltage

        return {
            'frequency_MHz': round(f_MHz, 2),
            'inductor_current_A': round(I_coil, 1),
            'inductor_current_phase_deg': round(phi_current, 1),
            'inductor_active_power_kW': round(P_coil / 1000, 2),
            'inductor_voltage_kV': round(V_coil / 1000, 2),
            'inductor_voltage_phase_deg': round(phi_voltage, 1),
            'plasma_current_A': round(plasma_current, 1),
            'plasma_current_phase_deg': round(phi_plasma_current, 1),
            'plasma_active_power_kW': round(total_power / 1000, 2),
            'transformation_ratio': round(plasma_current / I_coil, 3) if I_coil > 0 else 0,
            'effective_resistance_ohm': round(R_eff, 3),
            'effective_reactance_ohm': round(X_coil_ohm, 3),
            'quality_factor': round(Q_factor, 1)
        }


if __name__ == "__main__":
    root = tk.Tk()
    app = InductorPlasmaCalculatorGUI(root)
    root.mainloop()