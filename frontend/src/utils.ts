/**
 * Poll a condition every x MS.
 */
export function wait_for<T>(
  check: () => T,
  interval = 50
): Promise<NonNullable<T>> {
  return new Promise((resolve) => {
    let set: ReturnType<typeof setInterval>;

    const run = () => {
      try {
        const result = check();

        if (result) {
          if (set) clearInterval(set);
          resolve(result as NonNullable<T>);

          return true;
        }
      } catch (err) {
        console.error(err);
      }
    };

    if (!run()) set = setInterval(run, interval);
  });
}
