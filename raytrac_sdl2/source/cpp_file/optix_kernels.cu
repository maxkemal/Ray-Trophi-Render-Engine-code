// ... SBT olu�turma k�sm� ...
OptixShaderBindingTable sbt;
// ... SBT'yi konfig�re etme

// Hitgroup'ta sonu�lar� depola
extern "C" __global__ void __closesthit__radiance() {
    const HitGroupData* hitData = reinterpret_cast<HitGroupData*>(optixGetSbtData());
    // ... Di�er hesaplamalar

    // Sonucu SBT'nin payload b�l�m�ne yaz
    OptixHitResult* hitResult = reinterpret_cast<OptixHitResult*>(optixGetSbtDataPayload());
    hitResult->hasHit = true;
    // ... Di�er sonu�lar� ayarla
}

// Host'ta sonu�lar� oku
OptixHitResult* hostResult = new OptixHitResult[numRays];
cudaMemcpy(hostResult, sbt.payloadBase, numRays * sizeof(OptixHitResult), cudaMemcpyDeviceToHost);