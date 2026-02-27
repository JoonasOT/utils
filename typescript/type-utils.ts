//#region Simple types

type Not<B extends boolean> = B extends true ? false : true
type Prettify<T extends object> = {[k in keyof T]: T[k]} & {}
type NonEmptyArrayOf<T> = [T, ...T[]]
type StringWithHints<HINTS> = HINTS | (string & {})
type Is<T, SMT> = T extends SMT ? true : false
type If<T extends boolean, YES, NO> = T extends true ? YES : NO
type Or<B1 extends boolean, B2 extends boolean> = B1 extends true ? true : B2
type And<B1 extends boolean, B2 extends boolean> = B1 extends false ? false : B2
type Conditional<T, CHECK, YES, NO> = If<Is<T, CHECK>, YES, NO>
type Flip<T extends Record<PropertyKey, PropertyKey>> = {[k in keyof T as T[k]]: k}
type Prefix<UNION extends string, PREFIX extends string> = `${PREFIX}${UNION}`

type RequireProperties<OBJ extends object, KEYS extends keyof OBJ> = Required<Pick<OBJ, KEYS>> & Omit<OBJ, KEYS>
type PartialProperties<OBJ extends object, KEYS extends keyof OBJ> = Partial<Pick<OBJ, KEYS>> & Omit<OBJ, KEYS>
type ReadonlyProperties<OBJ extends object, KEYS extends keyof OBJ> = Readonly<Pick<OBJ, KEYS>> & Omit<OBJ, KEYS>

//#region String utils

type IgnoreEmptyStr<S extends string> = S extends "" ? never : S

type SplitString<S extends string, SEP extends string> = S extends `${infer FRONT}${SEP}${infer REST}` ? IgnoreEmptyStr<FRONT> | SplitString<REST, SEP> : IgnoreEmptyStr<S>

type JoinStrings<S extends string[], SEP extends string = ",", ACC extends string = "">
  = S extends [string, ...infer REST extends string[]] ?
    JoinStrings<REST, SEP, `${ACC}${SEP}${S[0]}`> : `${ACC}${SEP}${S[0]}` extends `,${infer JOINED extends string},undefined` ? JOINED : never

const mapFirstChar = <const S extends string, const MAPPED extends string>(str: S, map: (char: S[0]) => MAPPED) => (str.length ? `${map(str[0])}${str.length > 1 ? str.slice(1): ""}` : "")

const capitalize = <const T extends string>(str: T) => mapFirstChar(str, s => s.toLocaleUpperCase()) as Capitalize<T>
const uncapitalize = <const T extends string>(str: T) => mapFirstChar(str, s => s.toLocaleLowerCase()) as Uncapitalize<T>

type Quote<STR extends string, MARK extends string = '"'> = `${MARK}${STR}${MARK}`
const quote = <const STR extends string, const MARK extends "'" | '"'>(str: STR, mark: MARK = '"' as MARK) => `${mark}${str}${mark}` as Quote<STR, MARK>

type SnakeToCamelCase<S extends string> = S extends `${infer F}_${infer R}` ? `${F}${Capitalize<SnakeToCamelCase<R>>}` : S
const snakeToCamelCase = <const S extends string>(snake: S) => snake.toLowerCase().replace(/([-_][a-z])/g, rest => rest[1].toUpperCase()) as SnakeToCamelCase<S>

type CamelToSnakeCase<S extends string> = S extends `${infer T}${infer U}` ? T extends "_" ? `_${CamelToSnakeCase<U>}` : `${T extends Capitalize<T> ? "_" : ""}${Lowercase<T>}${CamelToSnakeCase<U>}` : S
const camelToSnakeCase = <const S extends string>(camel: S) => camel.replace(/[A-Z]/g, letter => `_${letter.toLowerCase()}`) as CamelToSnakeCase<S>

type PascalToSnakeCase<S extends string> = CamelToSnakeCase<Uncapitalize<S>>
const pascalToSnakeCase = <const S extends string>(pascal: S) => camelToSnakeCase(uncapitalize(pascal))

type SnakeToPascalCase<S extends string> = Capitalize<SnakeToCamelCase<S>>
const snakeToPascalCase = <const S extends string>(snake: S) => capitalize(snakeToCamelCase(snake)) as SnakeToPascalCase<S>

//#region Union utils

type IntersectUnion<UNION> = (UNION extends any ? (_: UNION) => never : never) extends ((_: infer INFERRED) => never) ? INFERRED : never
type IsUnion<T> = [T] extends [IntersectUnion<T>] ? false : true

type PopUnion<U> = IntersectUnion<
  U extends any ? (f: U) => void : never
> extends (a: infer A) => void ? A : never
type PopStringUnion<U> = IntersectUnion<
  U extends any ? (f: U) => void : never
> extends (a: infer A extends string) => void ? A : never

type UnionToArray<T, A extends unknown[] = []> = IsUnion<T> extends true
  ? UnionToArray<Exclude<T, PopUnion<T>>, [PopUnion<T>, ...A]>
  : [T, ...A]
type StringUnionToArray<T, A extends string[] = []> = IsUnion<T> extends true
  ? StringUnionToArray<Exclude<T, PopStringUnion<T>>, [PopStringUnion<T>, ...A]>
  : [T, ...A]

//#region Injection / Bijection

type IsInjection<T extends Record<PropertyKey, PropertyKey>> = Not<{[k in keyof Flip<T>]: IsUnion<Flip<T>[k]>}[keyof Flip<T>]>
type Bijectable<T extends Record<PropertyKey, PropertyKey>> = IsInjection<T> extends true ? T : never

const toBijection = <MAP extends Record<PropertyKey, PropertyKey>>(base: Bijectable<MAP>) => ({
  ...base,
  ...Object.fromEntries(Object.entries(base).map(([x,y]) => [y,x]))
}) as unknown as Prettify<MAP & Flip<MAP>>

//#region JSON Stringify

type __JsonQuote<S extends string> = Quote<S, "'">

type __JsonPrimitive = string | number | boolean | null
type __JsonValue =  __JsonPrimitive | __JsonRecord | __JsonArray
interface __JsonRecord extends Record<string | number, __JsonValue | undefined> {}
type __JsonArr = ReadonlyArray<__JsonValue>
interface __JsonArray extends __JsonArr {}

type __StringifyPrimitive<P extends __JsonPrimitive> = P extends string ? __JsonQuote<P> : `${P}`
type __StringifyUnion<U extends string, PRE extends string, POST extends string> = `${PRE}${JoinStrings<StringUnionToArray<U>>}${POST}`
type __StringifyKeyValuePair<KEY extends string | number, VALUE extends __JsonValue | undefined> = VALUE extends __JsonValue ? `${__StringifyPrimitive<KEY>}:${Stringify<VALUE>}` : ""
type __StringifyRecord<R extends __JsonRecord> = __StringifyUnion<Exclude<{[k in Exclude<keyof R, symbol>]: __StringifyKeyValuePair<k, R[k]>}[Exclude<keyof R, symbol>], "">, "{", "}">
type __StringifyArray<A extends __JsonArr> = `[${JoinStrings<[...{[v in keyof A]: `${Stringify<A[v]>}`}]>}]`

type Stringify<J extends __JsonValue> =
  J extends __JsonPrimitive ? __StringifyPrimitive<J> :
    J extends __JsonRecord ? __StringifyRecord<J> :
      J extends __JsonArr ? __StringifyArray<J> :
        never

//#region Nested Keys

type IfArray<T, YES, NO> = Conditional<T, any[], YES, NO>
type IsObjectOrArray<T> = Conditional<T, any[], true, Conditional<T, Record<PropertyKey, any>, true, false>>

type __FormatArray<SUB_TYPE, ROOT_NODE extends boolean> = (IsObjectOrArray<SUB_TYPE> extends true ? Prefix<`${IfArray<SUB_TYPE,"",".">}${NestedKey<SUB_TYPE, false>}`, `[${number}]`> : never) | `[${number}]` | `${Conditional<ROOT_NODE, true, "", ".">}length`
type __FormatObject<T extends Record<PropertyKey, unknown>, ROOT_NODE extends boolean> = {[k in Exclude<keyof T, symbol>]: Prefix<`${k}` | (IsObjectOrArray<T[k]> extends true ? `${k}${IfArray<T[k],"",".">}${NestedKey<T[k], false>}` : never), Conditional<ROOT_NODE, true, ".", "">>}[Exclude<keyof T, symbol>]

type NestedKey<T, ROOT_NODE extends boolean = true> =
  T extends (infer I)[] ? __FormatArray<I, ROOT_NODE> :
    T extends Record<PropertyKey, unknown> ? __FormatObject<T, ROOT_NODE> :
      never


type __NextIsArray<KEY extends string> = Conditional<KEY[0], "[", true, false>

type __RemoveArrayIndex<INDEX extends string> = INDEX extends `[${number}]${infer REST}` ? REST : INDEX

type __HandleRealizeArray<OBJ, KEY extends string, CURRY_UNDEF extends boolean> =
  OBJ extends (infer E)[] ?
    Or<Is<KEY, `[${number}].length`>, Is<KEY, ".length">> extends true ?
      OBJ["length"] | If<CURRY_UNDEF, undefined, never> :
      Conditional<KEY,  "", E | undefined, __RealizeNestedKeyImpl<E, __RemoveArrayIndex<KEY>, true>> :
    never
  
type __HandleRealizeObject<OBJ, OBJ_KEY extends string, KEY extends string, CURRY_UNDEF extends boolean> =
  OBJ_KEY extends keyof OBJ ?
    KEY extends `.${OBJ_KEY}${infer REST}` ?
      __RealizeNestedKeyImpl<OBJ[OBJ_KEY], REST, CURRY_UNDEF> :
      never :
    never

type __handleRealizeProperty<OBJ, OBJ_KEY extends string, KEY extends string, CURRY_UNDEF extends boolean> =
  OBJ_KEY extends keyof OBJ ?
    If<CURRY_UNDEF, OBJ[OBJ_KEY] | undefined, OBJ[OBJ_KEY]> :
    never

type __RealizeNestedKeyImpl<OBJ, KEY extends string, CURRY_UNDEF extends boolean> =
  KEY extends `[${number}]${string}` ? __HandleRealizeArray<OBJ, KEY, CURRY_UNDEF> :
      KEY extends `.${infer OBJ_KEY}${"." | "["}${string}` ? __HandleRealizeObject<OBJ, OBJ_KEY, KEY, CURRY_UNDEF> :
        KEY extends `.${infer OBJ_KEY}` ? __handleRealizeProperty<OBJ, OBJ_KEY, KEY, CURRY_UNDEF> :
          OBJ | If<CURRY_UNDEF, undefined, never>

type RealizeNestedKey<OBJ, KEY extends NestedKey<OBJ>> = __RealizeNestedKeyImpl<OBJ, KEY, Is<OBJ, any[]>>

const withNested = <const OBJ extends object>(obj: OBJ, key: NestedKey<OBJ>) => (() => {
  const getKeys = () => key
              .split(".")
              .flatMap(s => s.split("["))
              .flatMap(s => s.split("]"))
              .filter(s => !!s)

  const realize = (path: string[]) => path.reduce((prev, curr) => Array.isArray(prev) ? prev[+curr] : prev !== undefined ? prev[curr as keyof typeof prev] : prev, obj) as object | any[]

  return {
    get: () => realize(getKeys()) as RealizeNestedKey<OBJ, typeof key>,
    set: (value: RealizeNestedKey<OBJ, typeof key>) => {
      const path = getKeys(), key_ = path.pop()!, ob = realize(path)
      if (!ob) return false
      const old = ob[key_ as keyof typeof ob]
      ob[key_ as keyof typeof ob] = value as typeof old
      return true
    }
  }
})()

//#endregion