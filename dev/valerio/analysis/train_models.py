import numpy as np
import struct
import json
import os

def train():
    idx_path = "data/offsets.idx"
    if not os.path.exists(idx_path):
        print(f"Error: No se encontro el archivo de indices: {idx_path}")
        return

    print("[Trainer] Cargando indices binarios desde data/offsets.idx...")
    with open(idx_path, 'rb') as f:
        num_entries = struct.unpack('Q', f.read(8))[0]
        data = np.fromfile(f, dtype=[('id', 'u8'), ('offset', 'u8')], count=num_entries)

    keys = data['id'].astype(np.float64)
    N = len(keys)
    indices = np.arange(N, dtype=np.float64)

    print(f"[Trainer] Total registros cargados: {N}")

    # 1. Entrenar el modelo global de la Skip List (aproxima el indice [0, N-1])
    print("[Trainer] Entrenando modelo global para la Skip List...")
    a_global, b_global = np.polyfit(keys, indices, 1)

    # 2. Entrenar RMI de 2 etapas para el Arbol B* + ML
    print("[Trainer] Entrenando RMI Etapa 1...")
    a0, b0 = np.polyfit(keys, indices, 1)

    # Predecir indices aproximados y mapear a submodelos de la Etapa 2
    M = 1000  # Cantidad de submodelos en la Etapa 2
    pred_stage1 = a0 * keys + b0
    # Mapear al rango [0, M-1]
    model_indices = np.clip((pred_stage1 * (M / N)).astype(np.int32), 0, M - 1)

    print(f"[Trainer] Entrenando RMI Etapa 2 ({M} submodelos)...")
    stage2_models = []
    
    # Agrupar las claves e indices por submodelo
    for j in range(M):
        mask = (model_indices == j)
        j_keys = keys[mask]
        j_indices = indices[mask]

        if len(j_keys) > 1:
            aj, bj = np.polyfit(j_keys, j_indices, 1)
        elif len(j_keys) == 1:
            aj = 0.0
            bj = float(j_indices[0])
        else:
            aj = 0.0
            bj = 0.0 if len(stage2_models) == 0 else stage2_models[-1][1]

        stage2_models.append((float(aj), float(bj)))

    # Calcular el error maximo de prediccion de la RMI en la Etapa 2
    print("[Trainer] Calculando el error maximo absoluto (delta)...")
    
    # Hacemos el calculo vectorizado para mayor velocidad en Python
    aj_arr = np.array([m[0] for m in stage2_models])
    bj_arr = np.array([m[1] for m in stage2_models])
    
    pred_stage2 = aj_arr[model_indices] * keys + bj_arr[model_indices]
    errors = np.abs(pred_stage2 - indices)
    max_error = np.max(errors)

    # Redondear delta hacia arriba al siguiente entero
    delta = int(np.ceil(max_error))
    print(f"[Trainer] RMI completado. Delta (Error maximo absoluto) = {delta} elementos.")

    # Guardar coeficientes en un archivo JSON
    coef_dir = "data"
    os.makedirs(coef_dir, exist_ok=True)
    coef_path = os.path.join(coef_dir, "model_coefficients.json")
    
    output_data = {
        "a_global": float(a_global),
        "b_global": float(b_global),
        "a0": float(a0),
        "b0": float(b0),
        "stage2_models": stage2_models,
        "delta": delta,
        "M": M,
        "N": N
    }

    with open(coef_path, 'w') as f:
        json.dump(output_data, f)
    print(f"[Trainer] Coeficientes guardados en: {coef_path}")

    # Guardar en formato de texto simple para C++
    txt_path = os.path.join(coef_dir, "model_coefficients.txt")
    with open(txt_path, 'w') as f:
        f.write(f"{a_global} {b_global}\n")
        f.write(f"{a0} {b0} {delta} {M} {N}\n")
        for aj, bj in stage2_models:
            f.write(f"{aj} {bj}\n")
    print(f"[Trainer] Coeficientes de texto guardados en: {txt_path}")

if __name__ == "__main__":
    train()
