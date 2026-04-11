import json
import sys
import statistics

def find_anomalies(file_path):
    with open(file_path, 'r') as f:
        data = json.load(f)
        
    energies = [d['prime_energy_H0'] for d in data]
    mean = statistics.mean(energies)
    stdev = statistics.stdev(energies)
    
    print(f"Dataset Mean:  {mean:.4f}")
    print(f"Dataset Stdev: {stdev:.4f}")
    
    # Sort by absolute deviation from mean
    sorted_by_dev = sorted(data, key=lambda x: abs(x['prime_energy_H0'] - mean), reverse=True)
    
    print("\n[!] Top 15 Highest Deviations (Structural Anomalies):")
    for item in sorted_by_dev[:15]:
        target = item['target']
        e = item['prime_energy_H0']
        dev = abs(e - mean)
        dev_sigmas = dev / stdev if stdev > 0 else 0
        
        metrics = item['metrics']
        bd = metrics['branch_density']
        md = metrics['math_density']
        ls = metrics['load_store_ratio']
        cd = metrics['call_density']
        
        print(f" {target:<25} H0: {e:.4f} | +{dev_sigmas:.2f}σ | Br: {bd:.2f}, LS: {ls:.2f}, Math: {md:.2f}, Call: {cd:.2f}")

if __name__ == "__main__":
    find_anomalies("zcc_prime_features.json")
