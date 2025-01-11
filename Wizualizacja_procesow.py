import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from datetime import datetime

# Ścieżka do pliku logów
LOG_FILE = "cmake-build-debug/procesy.log"

# Funkcja do odczytu logów


def read_logs(file_path):
    processes = []
    try:
        with open(file_path, "r") as file:
            for line in file:
                if not line.strip():
                    continue

                # Rozbijamy linię na elementy
                parts = line.strip().split(" ")
                try:
                    # Sprawdź poprawność formatu
                    status = parts[0][1:-1]  # Usuwamy nawiasy []
                    pid = int(parts[1].split(":")[1])
                    typ = parts[2].split(":")[1]
                    id_ = int(parts[3].split(":")[1])
                    timestamp = parts[4].split(":")[1] + " " + parts[5]  # Łączymy datę i czas

                    # Przetwarzanie czasu
                    time_obj = datetime.strptime(timestamp, "%Y-%m-%d %H:%M:%S")
                    processes.append({
                        "status": status,
                        "pid": pid,
                        "type": typ,
                        "id": id_,
                        "time": time_obj
                    })
                except (IndexError, ValueError) as e:
                    print(f"Błąd parsowania czasu w linii: {line.strip()}. Błąd: {e}")
    except FileNotFoundError:
        print(f"Błąd: Nie znaleziono pliku {file_path}.")
    except Exception as e:
        print(f"Błąd podczas odczytu pliku: {e}")
    return processes

# Funkcja do wizualizacji logów
def visualize_logs(processes):
    if not processes:
        print("Brak danych do wizualizacji.")
        return

    processes_by_pid = {}
    for entry in processes:
        pid = entry["pid"]
        if pid not in processes_by_pid:
            processes_by_pid[pid] = []
        processes_by_pid[pid].append(entry)

    fig, ax = plt.subplots(figsize=(12, 6))
    colors = {"START": "green", "END": "red"}
    y_positions = {}
    current_y = 0

    for pid, entries in processes_by_pid.items():
        y_positions[pid] = current_y
        for entry in entries:
            color = colors.get(entry["status"], "blue")
            ax.scatter(entry["time"], current_y, color=color, label=entry["status"])
        current_y += 1

    # Dodanie legendy
    patches = [mpatches.Patch(color=color, label=status) for status, color in colors.items()]
    ax.legend(handles=patches, loc="upper right")

    # Ustawienia osi
    ax.set_yticks(list(y_positions.values()))
    ax.set_yticklabels([f"PID {pid}" for pid in y_positions.keys()])
    ax.set_xlabel("Czas")
    ax.set_ylabel("Procesy")
    ax.set_title("Wizualizacja procesów na podstawie logów")

    plt.tight_layout()
    plt.show()

# Główna logika
def main():
    processes = read_logs(LOG_FILE)
    visualize_logs(processes)

if __name__ == "__main__":
    main()