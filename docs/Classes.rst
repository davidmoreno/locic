Classes
=======

The ability to create and manipulate objects, which are instances of a class, provides a useful way to encapsulate a set of behaviour around a particular data type, and maintaining the necessary invariants needed by that type. Therefore Loci, like many other languages, includes support for object oriented programming.

Definitions
-----------

Here's an example of a Loci class definition:

.. code-block:: c++

	class Bucket(bool b, int value) {
		static create = default;
		
		bool containsValue() const {
			return @b;
		}
		
		void placeValue(int value) {
			assert !@b;
			@value = value;
		}
		
		int getValue() const {
			assert @b;
			return @value;
		}
	}

The organisation is somewhat different to many languages, and member variables are defined within the parentheses after the class name. The methods, constructors and destructors then appear within the braces that follow. (Also note the use of the :doc:`Assert Statement <AssertStatement>`.)

Another detail is that instance member variables are referred to using the @ symbol, which provides a clean alternative to the various C++ conventions for naming such variables (for example: 'mValue' or 'value\_').

Constructors
------------

The above example shows a class with a default constructor named *create*, which is automatically generated by the compiler to be equivalent to:
 
.. code-block:: c++

	class Bucket(bool b, int value){
		static Bucket create(bool b, int value) {
			return @(move b, move value);
		}
		
		// etc.
	}
	
	void function(){
		// Equivalent ways to call the default constructor:
		
		// Implicitly call.
		Bucket bucket = Bucket(true, 1);
		
		// Direct name reference.
		Bucket bucket = Bucket::create(true, 1);
		
		// Calling via typename staticref.
		Bucket bucket = Bucket.create(true, 1);
	}

Here the *@* symbol being called is the *internal constructor*, and performs the actual construction of the class type, which is the process of filling the fields of the struct underlying the class with the values given to it. This system is intended as a superior alternative to initialization lists in C++ and Java. Here's an example of a C++ initialization list:


.. code-block:: c++

	// This is C++ code.
	class Bucket{
		public:
			Bucket(bool b, int value)
				: b_(b), value_(value){ }
	};

Avoiding initialization lists makes it easy to express complex logic in constructors, such as loops:

.. code-block:: c++

	class SomeClass(int value0, int value1) {
		static Factorial(int n) {
			int r = 1;
			for (int i = 2; i < n; i++) {
				r *= i;
			}
			return @(r, r * r);
		}
	}
	
	void function(){
		SomeClass c = SomeClass::Factorial(10);
	}

Other named constructors can also be created:

.. code-block:: c++

	class Bucket(bool b, int value) {
		// ...
		
		static WithNoValue() {
			return @(false, 0);
		}
		
		static WithZeroValue() {
			return @(true, 0);
		}
	
		// ...
	}
	
	void function(){
		Bucket emptyBucket = Bucket::WithNoValue();
		Bucket zeroBucket = Bucket::WithZeroValue();
	}

Declarations
------------

Perhaps most interesting is the ability to declare classes. Unlike C++ where this is commonly needed, declarations typically won't be needed within a shared codebase, because Loci can find the class definitions and produce their corresponding declarations automatically.

Additionally, Loci uses multiple pass compilation so there is no requirement to define types or functions before they are used, and hence there is no need to declare a class which is later defined in the same code base, something that is common in C and C++.

However, if a class is to be used across an API boundary (for example, the class is defined inside a static library, and a program uses the declaration to create instances of the class and call its methods) then declarations should be used.

Here's a declaration of the Bucket class:

.. code-block:: c++

	class Bucket {
		static Bucket create(bool b, int v);
	
		bool containsValue() const;
		
		void placeValue(int value);
		
		int getValue() const;
	}

Clearly, the class method implementations have been removed, however the member variables have also been removed. This is because, unlike C++, class declarations in Loci do not need to include the types (or names) of the member variables.

From a design perspective, this prevents the need to use forward struct declarations (or similar) and heap allocations that are needed in C and C++, and is part of fulfilling Loci's goal to be an excellent language for defining, implementing and using APIs. Therefore clients of a library do not need to be recompiled when a class in the library changes its member variables, since this is private information.

Also note that the auto-generated default constructor has to be specified explicitly here, since users of the class declaration have no knowledge of the types (or number, or order) of the member variables and therefore cannot know the type of the constructor.

This and Self
-------------

Methods can use the keywords *this* and *self* to obtain a pointer or reference (respectively) to their object. Here's an example:

.. code-block:: c++

	class ExampleClass() {
		// ...
		
		ExampleClass* getThis() {
			return this;
		}
		
		ExampleClass& getSelf() {
			return self;
		}
	
		// ...
	}

