/**
 * TohruPhysics demo — verifies the library links and runs.
 * TohruPhysicsのデモ — ライブラリがリンクして動作することを確認するの。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Arena.h>
#include <TohruPhysics/Vector3.h>
#include <stdio.h>

int main(void) {
	Arena A;
	Error Err = TohruArenaInit(&A, 1024);
	if (ErrIsFail(Err)) {
		fprintf(stderr, "Arena init failed\n");
		return 1;
	}

	Vector3 V = KannaVector3Make(1.0, 2.0, 3.0);
	Vector3 N = KannaVector3Normalize(&V);
	fprintf(stderr, "Demo: normalized (%f, %f, %f)\n",
		N.Data[0], N.Data[1], N.Data[2]);

	TohruArenaDestroy(&A);
	fprintf(stderr, "Demo: OK\n");
	return 0;
}
